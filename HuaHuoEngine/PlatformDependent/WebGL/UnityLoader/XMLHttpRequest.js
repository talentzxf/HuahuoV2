  Module.XMLHttpRequest = function () {
    var UnityCacheDatabase = { name: "UnityCache", version: 2 };
    var XMLHttpRequestStore = { name: "XMLHttpRequest", version: 1 };
    var WebAssemblyStore = { name: "WebAssembly", version: 1 };
    
    function log(message) {
      console.log("[UnityCache] " + message);
    }
    
    function resolveURL(url) {
      resolveURL.link = resolveURL.link || document.createElement("a");
      resolveURL.link.href = url;
      return resolveURL.link.href;
    }
    
    function isCrossOriginURL(url) {
      var originMatch = window.location.href.match(/^[a-z]+:\/\/[^\/]+/);
      return !originMatch || url.lastIndexOf(originMatch[0], 0);
    }

    function UnityCache() {
      var cache = this;
      cache.queue = [];

      function initDatabase(database) {
        if (typeof cache.database != "undefined")
          return;
        cache.database = database;
        if (!cache.database)
          log("indexedDB database could not be opened");
        while (cache.queue.length) {
          var queued = cache.queue.shift();
          if (cache.database) {
            cache.execute.apply(cache, queued);
          } else if (typeof queued.onerror == "function") {
            queued.onerror(new Error("operation cancelled"));
          }
        }
      }
      
      try {
        var indexedDB = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
        function upgradeDatabase() {
          var openRequest = indexedDB.open(UnityCacheDatabase.name, UnityCacheDatabase.version);
          openRequest.onupgradeneeded = function (e) {
            var database = e.target.result;
            if (!database.objectStoreNames.contains(WebAssemblyStore.name))
              database.createObjectStore(WebAssemblyStore.name);
          };
          openRequest.onsuccess = function (e) { initDatabase(e.target.result); };
          openRequest.onerror = function () { initDatabase(null); };
        }
        var openRequest = indexedDB.open(UnityCacheDatabase.name);
        openRequest.onupgradeneeded = function (e) {
          var objectStore = e.target.result.createObjectStore(XMLHttpRequestStore.name, { keyPath: "url" });
          ["version", "company", "product", "updated", "revalidated", "accessed"].forEach(function (index) { objectStore.createIndex(index, index); });
        };
        openRequest.onsuccess = function (e) {
          var database = e.target.result;
          if (database.version < UnityCacheDatabase.version) {
            database.close();
            upgradeDatabase();
          } else {
            initDatabase(database);
          }
        };
        openRequest.onerror = function () { initDatabase(null); };
      } catch (e) {
        initDatabase(null);
      }
    };
    
    UnityCache.prototype.execute = function (store, operation, parameters, onsuccess, onerror) {
      if (this.database) {
        try {
          var target = this.database.transaction([store], ["put", "delete", "clear"].indexOf(operation) != -1 ? "readwrite" : "readonly").objectStore(store);
          if (operation == "openKeyCursor") {
            target = target.index(parameters[0]);
            parameters = parameters.slice(1);
          }
          var request = target[operation].apply(target, parameters);
          if (typeof onsuccess == "function")
            request.onsuccess = function (e) { onsuccess(e.target.result); };
          request.onerror = onerror;
        } catch (e) {
          if (typeof onerror == "function")
            onerror(e);
        }
      } else if (typeof this.database == "undefined") {
        this.queue.push(arguments);
      } else if (typeof onerror == "function") {
        onerror(new Error("indexedDB access denied"));
      }
    };
    
    var unityCache = new UnityCache();
    
    function createXMLHttpRequestResult(url, company, product, timestamp, xhr) {
      var result = { url: url, version: XMLHttpRequestStore.version, company: company, product: product, updated: timestamp, revalidated: timestamp, accessed: timestamp, responseHeaders: {}, xhr: {} };
      if (xhr) {
        ["Last-Modified", "ETag"].forEach(function (header) { result.responseHeaders[header] = xhr.getResponseHeader(header); });
        ["responseURL", "status", "statusText", "response"].forEach(function (property) { result.xhr[property] = xhr[property]; });
      }
      return result;
    }

    function CachedXMLHttpRequest(objParameters) {
      this.cache = { enabled: false };
      if (objParameters) {
        this.cache.control = objParameters.cacheControl;
        this.cache.company = objParameters.companyName;
        this.cache.product = objParameters.productName;
      }
      this.xhr = new XMLHttpRequest(objParameters);
      this.xhr.addEventListener("load", function () {
        var xhr = this.xhr, cache = this.cache;
        if (!cache.enabled || cache.revalidated)
          return;
        if (xhr.status == 304) {
          cache.result.revalidated = cache.result.accessed;
          cache.revalidated = true;
          unityCache.execute(XMLHttpRequestStore.name, "put", [cache.result]);
          log("'" + cache.result.url + "' successfully revalidated and served from the indexedDB cache");
        } else if (xhr.status == 200) {
          cache.result = createXMLHttpRequestResult(cache.result.url, cache.company, cache.product, cache.result.accessed, xhr);
          cache.revalidated = true;
          unityCache.execute(XMLHttpRequestStore.name, "put", [cache.result], function (result) {
            log("'" + cache.result.url + "' successfully downloaded and stored in the indexedDB cache");
          }, function (error) {
            log("'" + cache.result.url + "' successfully downloaded but not stored in the indexedDB cache due to the error: " + error);
          });
        } else {
          log("'" + cache.result.url + "' request failed with status: " + xhr.status + " " + xhr.statusText);
        }
      }.bind(this));
    };
    
    CachedXMLHttpRequest.prototype.send = function (data) {
      var xhr = this.xhr, cache = this.cache;
      var sendArguments = arguments;
      cache.enabled = cache.enabled && xhr.responseType == "arraybuffer" && !data;
      if (!cache.enabled)
        return xhr.send.apply(xhr, sendArguments);
      unityCache.execute(XMLHttpRequestStore.name, "get", [cache.result.url], function (result) {
        if (!result || result.version != XMLHttpRequestStore.version) {
          xhr.send.apply(xhr, sendArguments);
          return;
        }
        cache.result = result;
        cache.result.accessed = Date.now();
        if (cache.control == "immutable") {
          cache.revalidated = true;
          unityCache.execute(XMLHttpRequestStore.name, "put", [cache.result]);
          xhr.dispatchEvent(new Event('load'));
          log("'" + cache.result.url + "' served from the indexedDB cache without revalidation");
        } else if (isCrossOriginURL(cache.result.url) && (cache.result.responseHeaders["Last-Modified"] || cache.result.responseHeaders["ETag"])) {
          var headXHR = new XMLHttpRequest();
          headXHR.open("HEAD", cache.result.url);
          headXHR.onload = function () {
            cache.revalidated = ["Last-Modified", "ETag"].every(function (header) {
              return !cache.result.responseHeaders[header] || cache.result.responseHeaders[header] == headXHR.getResponseHeader(header);
            });
            if (cache.revalidated) {
              cache.result.revalidated = cache.result.accessed;
              unityCache.execute(XMLHttpRequestStore.name, "put", [cache.result]);
              xhr.dispatchEvent(new Event('load'));
              log("'" + cache.result.url + "' successfully revalidated and served from the indexedDB cache");
            } else {
              xhr.send.apply(xhr, sendArguments);
            }
          }
          headXHR.send();
        } else {
          if (cache.result.responseHeaders["Last-Modified"]) {
            xhr.setRequestHeader("If-Modified-Since", cache.result.responseHeaders["Last-Modified"]);
            xhr.setRequestHeader("Cache-Control", "no-cache");
          } else if (cache.result.responseHeaders["ETag"]) {
            xhr.setRequestHeader("If-None-Match", cache.result.responseHeaders["ETag"]);
            xhr.setRequestHeader("Cache-Control", "no-cache");
          }
          xhr.send.apply(xhr, sendArguments);
        }
      }, function (error) {
        xhr.send.apply(xhr, sendArguments);
      });
    };
    
    CachedXMLHttpRequest.prototype.open = function (method, url, async, user, password) {
      this.cache.result = createXMLHttpRequestResult(resolveURL(url), this.cache.company, this.cache.product, Date.now());
      this.cache.enabled = ["must-revalidate", "immutable"].indexOf(this.cache.control) != -1 && method == "GET" && this.cache.result.url.match("^https?:\/\/")
        && (typeof async == "undefined" || async) && typeof user == "undefined" && typeof password == "undefined";
      this.cache.revalidated = false;
      return this.xhr.open.apply(this.xhr, arguments);
    };
    
    CachedXMLHttpRequest.prototype.setRequestHeader = function (header, value) {
      this.cache.enabled = false;
      return this.xhr.setRequestHeader.apply(this.xhr, arguments);
    };
    
    var xhr = new XMLHttpRequest();
    for (var property in xhr) {
      if (!CachedXMLHttpRequest.prototype.hasOwnProperty(property)) {
        (function (property) {
          Object.defineProperty(CachedXMLHttpRequest.prototype, property, typeof xhr[property] == "function" ? {
            value: function () { return this.xhr[property].apply(this.xhr, arguments); },
          } : {
            get: function () { return this.cache.revalidated && this.cache.result.xhr.hasOwnProperty(property) ? this.cache.result.xhr[property] : this.xhr[property]; },
            set: function (value) { this.xhr[property] = value; },
          });
        })(property);
      }
    }
    
    return CachedXMLHttpRequest;
  } ();
