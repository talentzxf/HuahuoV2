// Cherry-pick patches from upstream Emscripten:
// 1. Added support for EXT_texture_norm16
// 2. Added support for WEBKIT_WEBGL_compressed_texture_pvrtc
// 3. Always create high-performance WebGL contexts
// TODO: Delete this file when updating Emscripten to newer version.

/*
 * GL support. See http://kripken.github.io/emscripten-site/docs/porting/multimedia_and_graphics/OpenGL-support.html
 * for current status.
 */

var LibraryGL = {
#if USE_PTHREADS
  // If GLctxIsOnParentThread is true, then
  //   a) the running thread is not the main browser thread, and
  //   b) the currently active GL context is running on the main browser thread, so all GL calls need to be proxied to it.
  $GL__postset: 'var GLctx; var GLctxIsOnParentThread = false; GL.init()',
#else
  $GL__postset: 'var GLctx; GL.init()',
#endif
  $GL: {
#if GL_DEBUG
    debug: true,
#endif

    counter: 1, // 0 is reserved as 'null' in gl
    lastError: 0,
    buffers: [],
    mappedBuffers: {},
    programs: [],
    framebuffers: [],
    renderbuffers: [],
    textures: [],
    uniforms: [],
    shaders: [],
    vaos: [],
    contexts: [],
    currentContext: null,
    offscreenCanvases: {}, // DOM ID -> OffscreenCanvas mappings of <canvas> elements that have their rendering control transferred to offscreen.
    timerQueriesEXT: [],
#if USE_WEBGL2
    queries: [],
    samplers: [],
    transformFeedbacks: [],
    syncs: [],
#endif

#if USES_GL_EMULATION
    currArrayBuffer: 0,
    currElementArrayBuffer: 0,
#endif

    byteSizeByTypeRoot: 0x1400, // GL_BYTE
    byteSizeByType: [
      1, // GL_BYTE
      1, // GL_UNSIGNED_BYTE
      2, // GL_SHORT
      2, // GL_UNSIGNED_SHORT
      4, // GL_INT
      4, // GL_UNSIGNED_INT
      4, // GL_FLOAT
      2, // GL_2_BYTES
      3, // GL_3_BYTES
      4, // GL_4_BYTES
      8  // GL_DOUBLE
    ],

    programInfos: {}, // Stores additional information needed for each shader program. Each entry is of form:
    /* { uniforms: {}, // Maps ints back to the opaque WebGLUniformLocation objects.
         maxUniformLength: int, // Cached in order to implement glGetProgramiv(GL_ACTIVE_UNIFORM_MAX_LENGTH)
         maxAttributeLength: int // Cached in order to implement glGetProgramiv(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH)
         maxUniformBlockNameLength: int // Cached in order to implement glGetProgramiv(GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH)
       } */

    stringCache: {},
#if USE_WEBGL2
    stringiCache: {},
#endif
    tempFixedLengthArray: [],

    packAlignment: 4,   // default alignment is 4 bytes
    unpackAlignment: 4, // default alignment is 4 bytes

    init: function() {
#if USES_GL_EMULATION
      GL.createLog2ceilLookup(GL.MAX_TEMP_BUFFER_SIZE);
#endif
      GL.miniTempBuffer = new Float32Array(GL.MINI_TEMP_BUFFER_SIZE);
      for (var i = 0; i < GL.MINI_TEMP_BUFFER_SIZE; i++) {
        GL.miniTempBufferViews[i] = GL.miniTempBuffer.subarray(0, i+1);
      }

      // For functions such as glDrawBuffers, glInvalidateFramebuffer and glInvalidateSubFramebuffer that need to pass a short array to the WebGL API,
      // create a set of short fixed-length arrays to avoid having to generate any garbage when calling those functions.
      for (var i = 0; i < 32; i++) {
        GL.tempFixedLengthArray.push(new Array(i));
      }
    },

    // Records a GL error condition that occurred, stored until user calls glGetError() to fetch it. As per GLES2 spec, only the first error
    // is remembered, and subsequent errors are discarded until the user has cleared the stored error by a call to glGetError().
    recordError: function recordError(errorCode) {
      if (!GL.lastError) {
        GL.lastError = errorCode;
      }
    },
    // Get a new ID for a texture/buffer/etc., while keeping the table dense and fast. Creation is fairly rare so it is worth optimizing lookups later.
    getNewId: function(table) {
      var ret = GL.counter++;
      for (var i = table.length; i < ret; i++) {
        table[i] = null;
      }
      return ret;
    },

    // Mini temp buffer
    MINI_TEMP_BUFFER_SIZE: 256,
    miniTempBuffer: null,
    miniTempBufferViews: [0], // index i has the view of size i+1

#if USES_GL_EMULATION
    // When user GL code wants to render from client-side memory, we need to upload the vertex data to a temp VBO
    // for rendering. Maintain a set of temp VBOs that are created-on-demand to appropriate sizes, and never destroyed.
    // Also, for best performance the VBOs are double-buffered, i.e. every second frame we switch the set of VBOs we
    // upload to, so that rendering from the previous frame is not disturbed by uploading from new data to it, which
    // could cause a GPU-CPU pipeline stall.
    // Note that index buffers are not double-buffered (at the moment) in this manner.
    MAX_TEMP_BUFFER_SIZE: {{{ GL_MAX_TEMP_BUFFER_SIZE }}},
    // Maximum number of temp VBOs of one size to maintain, after that we start reusing old ones, which is safe but can give
    // a performance impact. If CPU-GPU stalls are a problem, increasing this might help.
    numTempVertexBuffersPerSize: 64, // (const)

    // Precompute a lookup table for the function ceil(log2(x)), i.e. how many bits are needed to represent x, or,
    // if x was rounded up to next pow2, which index is the single '1' bit at?
    // Then log2ceilLookup[x] returns ceil(log2(x)).
    log2ceilLookup: null,
    createLog2ceilLookup: function(maxValue) {
      GL.log2ceilLookup = new Uint8Array(maxValue+1);
      var log2 = 0;
      var pow2 = 1;
      GL.log2ceilLookup[0] = 0;
      for (var i = 1; i <= maxValue; ++i) {
        if (i > pow2) {
          pow2 <<= 1;
          ++log2;
        }
        GL.log2ceilLookup[i] = log2;
      }
    },

    generateTempBuffers: function(quads, context) {
      var largestIndex = GL.log2ceilLookup[GL.MAX_TEMP_BUFFER_SIZE];
      context.tempVertexBufferCounters1 = [];
      context.tempVertexBufferCounters2 = [];
      context.tempVertexBufferCounters1.length = context.tempVertexBufferCounters2.length = largestIndex+1;
      context.tempVertexBuffers1 = [];
      context.tempVertexBuffers2 = [];
      context.tempVertexBuffers1.length = context.tempVertexBuffers2.length = largestIndex+1;
      context.tempIndexBuffers = [];
      context.tempIndexBuffers.length = largestIndex+1;
      for (var i = 0; i <= largestIndex; ++i) {
        context.tempIndexBuffers[i] = null; // Created on-demand
        context.tempVertexBufferCounters1[i] = context.tempVertexBufferCounters2[i] = 0;
        var ringbufferLength = GL.numTempVertexBuffersPerSize;
        context.tempVertexBuffers1[i] = [];
        context.tempVertexBuffers2[i] = [];
        var ringbuffer1 = context.tempVertexBuffers1[i];
        var ringbuffer2 = context.tempVertexBuffers2[i];
        ringbuffer1.length = ringbuffer2.length = ringbufferLength;
        for (var j = 0; j < ringbufferLength; ++j) {
          ringbuffer1[j] = ringbuffer2[j] = null; // Created on-demand
        }
      }

      if (quads) {
        // GL_QUAD indexes can be precalculated
        context.tempQuadIndexBuffer = GLctx.createBuffer();
        context.GLctx.bindBuffer(context.GLctx.ELEMENT_ARRAY_BUFFER, context.tempQuadIndexBuffer);
        var numIndexes = GL.MAX_TEMP_BUFFER_SIZE >> 1;
        var quadIndexes = new Uint16Array(numIndexes);
        var i = 0, v = 0;
        while (1) {
          quadIndexes[i++] = v;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v+1;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v+2;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v+2;
          if (i >= numIndexes) break;
          quadIndexes[i++] = v+3;
          if (i >= numIndexes) break;
          v += 4;
        }
        context.GLctx.bufferData(context.GLctx.ELEMENT_ARRAY_BUFFER, quadIndexes, context.GLctx.STATIC_DRAW);
        context.GLctx.bindBuffer(context.GLctx.ELEMENT_ARRAY_BUFFER, null);
      }
    },

    getTempVertexBuffer: function getTempVertexBuffer(sizeBytes) {
      var idx = GL.log2ceilLookup[sizeBytes];
      var ringbuffer = GL.currentContext.tempVertexBuffers1[idx];
      var nextFreeBufferIndex = GL.currentContext.tempVertexBufferCounters1[idx];
      GL.currentContext.tempVertexBufferCounters1[idx] = (GL.currentContext.tempVertexBufferCounters1[idx]+1) & (GL.numTempVertexBuffersPerSize-1);
      var vbo = ringbuffer[nextFreeBufferIndex];
      if (vbo) {
        return vbo;
      }
      var prevVBO = GLctx.getParameter(GLctx.ARRAY_BUFFER_BINDING);
      ringbuffer[nextFreeBufferIndex] = GLctx.createBuffer();
      GLctx.bindBuffer(GLctx.ARRAY_BUFFER, ringbuffer[nextFreeBufferIndex]);
      GLctx.bufferData(GLctx.ARRAY_BUFFER, 1 << idx, GLctx.DYNAMIC_DRAW);
      GLctx.bindBuffer(GLctx.ARRAY_BUFFER, prevVBO);
      return ringbuffer[nextFreeBufferIndex];
    },

    getTempIndexBuffer: function getTempIndexBuffer(sizeBytes) {
      var idx = GL.log2ceilLookup[sizeBytes];
      var ibo = GL.currentContext.tempIndexBuffers[idx];
      if (ibo) {
        return ibo;
      }
      var prevIBO = GLctx.getParameter(GLctx.ELEMENT_ARRAY_BUFFER_BINDING);
      GL.currentContext.tempIndexBuffers[idx] = GLctx.createBuffer();
      GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, GL.currentContext.tempIndexBuffers[idx]);
      GLctx.bufferData(GLctx.ELEMENT_ARRAY_BUFFER, 1 << idx, GLctx.DYNAMIC_DRAW);
      GLctx.bindBuffer(GLctx.ELEMENT_ARRAY_BUFFER, prevIBO);
      return GL.currentContext.tempIndexBuffers[idx];
    },

    // Called at start of each new WebGL rendering frame. This swaps the doublebuffered temp VB memory pointers,
    // so that every second frame utilizes different set of temp buffers. The aim is to keep the set of buffers
    // being rendered, and the set of buffers being updated disjoint.
    newRenderingFrameStarted: function newRenderingFrameStarted() {
      if (!GL.currentContext) {
        return;
      }
      var vb = GL.currentContext.tempVertexBuffers1;
      GL.currentContext.tempVertexBuffers1 = GL.currentContext.tempVertexBuffers2;
      GL.currentContext.tempVertexBuffers2 = vb;
      vb = GL.currentContext.tempVertexBufferCounters1;
      GL.currentContext.tempVertexBufferCounters1 = GL.currentContext.tempVertexBufferCounters2;
      GL.currentContext.tempVertexBufferCounters2 = vb;
      var largestIndex = GL.log2ceilLookup[GL.MAX_TEMP_BUFFER_SIZE];
      for (var i = 0; i <= largestIndex; ++i) {
        GL.currentContext.tempVertexBufferCounters1[i] = 0;
      }
    },
#endif

#if LEGACY_GL_EMULATION
    // Find a token in a shader source string
    findToken: function(source, token) {
      function isIdentChar(ch) {
        if (ch >= 48 && ch <= 57) // 0-9
          return true;
        if (ch >= 65 && ch <= 90) // A-Z
          return true;
        if (ch >= 97 && ch <= 122) // a-z
          return true;
        return false;
      }
      var i = -1;
      do {
        i = source.indexOf(token, i + 1);
        if (i < 0) {
          break;
        }
        if (i > 0 && isIdentChar(source[i - 1])) {
          continue;
        }
        i += token.length;
        if (i < source.length - 1 && isIdentChar(source[i + 1])) {
          continue;
        }
        return true;
      } while (true);
      return false;
    },
#endif

    getSource: function(shader, count, string, length) {
      var source = '';
      for (var i = 0; i < count; ++i) {
        var frag;
        if (length) {
          var len = {{{ makeGetValue('length', 'i*4', 'i32') }}};
          if (len < 0) {
            frag = Pointer_stringify({{{ makeGetValue('string', 'i*4', 'i32') }}});
          } else {
            frag = Pointer_stringify({{{ makeGetValue('string', 'i*4', 'i32') }}}, len);
          }
        } else {
          frag = Pointer_stringify({{{ makeGetValue('string', 'i*4', 'i32') }}});
        }
        source += frag;
      }
#if LEGACY_GL_EMULATION
      // Let's see if we need to enable the standard derivatives extension
      var type = GLctx.getShaderParameter(GL.shaders[shader], 0x8B4F /* GL_SHADER_TYPE */);
      if (type == 0x8B30 /* GL_FRAGMENT_SHADER */) {
        if (GL.findToken(source, "dFdx") ||
            GL.findToken(source, "dFdy") ||
            GL.findToken(source, "fwidth")) {
          source = "#extension GL_OES_standard_derivatives : enable\n" + source;
          var extension = GLctx.getExtension("OES_standard_derivatives");
#if GL_DEBUG
          if (!extension) {
            err("Shader attempts to use the standard derivatives extension which is not available.");
          }
#endif
        }
      }
#endif
      return source;
    },

#if GL_FFP_ONLY
    enabledClientAttribIndices: [],
    enableVertexAttribArray: function enableVertexAttribArray(index) {
      if (!GL.enabledClientAttribIndices[index]) {
        GL.enabledClientAttribIndices[index] = true;
        GLctx.enableVertexAttribArray(index);
      }
    },
    disableVertexAttribArray: function disableVertexAttribArray(index) {
      if (GL.enabledClientAttribIndices[index]) {
        GL.enabledClientAttribIndices[index] = false;
        GLctx.disableVertexAttribArray(index);
      }
    },
#endif

#if FULL_ES2
    calcBufLength: function calcBufLength(size, type, stride, count) {
      if (stride > 0) {
        return count * stride;  // XXXvlad this is not exactly correct I don't think
      }
      var typeSize = GL.byteSizeByType[type - GL.byteSizeByTypeRoot];
      return size * typeSize * count;
    },

    usedTempBuffers: [],

    preDrawHandleClientVertexAttribBindings: function preDrawHandleClientVertexAttribBindings(count) {
      GL.resetBufferBinding = false;

      // TODO: initial pass to detect ranges we need to upload, might not need an upload per attrib
      for (var i = 0; i < GL.currentContext.maxVertexAttribs; ++i) {
        var cb = GL.currentContext.clientBuffers[i];
        if (!cb.clientside || !cb.enabled) continue;

        GL.resetBufferBinding = true;

        var size = GL.calcBufLength(cb.size, cb.type, cb.stride, count);
        var buf = GL.getTempVertexBuffer(size);
        GLctx.bindBuffer(GLctx.ARRAY_BUFFER, buf);
        GLctx.bufferSubData(GLctx.ARRAY_BUFFER,
                                 0,
                                 HEAPU8.subarray(cb.ptr, cb.ptr + size));
#if GL_ASSERTIONS
        GL.validateVertexAttribPointer(cb.size, cb.type, cb.stride, 0);
#endif
        GLctx.vertexAttribPointer(i, cb.size, cb.type, cb.normalized, cb.stride, 0);
      }
    },

    postDrawHandleClientVertexAttribBindings: function postDrawHandleClientVertexAttribBindings() {
      if (GL.resetBufferBinding) {
        GLctx.bindBuffer(GLctx.ARRAY_BUFFER, GL.buffers[GL.currArrayBuffer]);
      }
    },
#endif

#if GL_ASSERTIONS
    validateGLObjectID: function(objectHandleArray, objectID, callerFunctionName, objectReadableType) {
      if (objectID != 0) {
        if (objectHandleArray[objectID] === null) {
          console.error(callerFunctionName + ' called with an already deleted ' + objectReadableType + ' ID ' + objectID + '!');
        } else if (!objectHandleArray[objectID]) {
          console.error(callerFunctionName + ' called with an invalid ' + objectReadableType + ' ID ' + objectID + '!');
        }
      }
    },
    // Validates that user obeys GL spec #6.4: http://www.khronos.org/registry/webgl/specs/latest/1.0/#6.4
    validateVertexAttribPointer: function(dimension, dataType, stride, offset) {
      var sizeBytes = 1;
      switch(dataType) {
        case 0x1400 /* GL_BYTE */:
        case 0x1401 /* GL_UNSIGNED_BYTE */:
          sizeBytes = 1;
          break;
        case 0x1402 /* GL_SHORT */:
        case 0x1403 /* GL_UNSIGNED_SHORT */:
          sizeBytes = 2;
          break;
        case 0x1404 /* GL_INT */:
        case 0x1405 /* GL_UNSIGNED_INT */:
        case 0x1406 /* GL_FLOAT */:
          sizeBytes = 4;
          break;
        case 0x140A /* GL_DOUBLE */:
          sizeBytes = 8;
          break;
        default:
          console.error('Invalid vertex attribute data type GLenum ' + dataType + ' passed to GL function!');
      }
      if (dimension == 0x80E1 /* GL_BGRA */) {
        console.error('WebGL does not support size=GL_BGRA in a call to glVertexAttribPointer! Please use size=4 and type=GL_UNSIGNED_BYTE instead!');
      } else if (dimension < 1 || dimension > 4) {
        console.error('Invalid dimension='+dimension+' in call to glVertexAttribPointer, must be 1,2,3 or 4.');
      }
      if (stride < 0 || stride > 255) {
        console.error('Invalid stride='+stride+' in call to glVertexAttribPointer. Note that maximum supported stride in WebGL is 255!');
      }
      if (offset % sizeBytes != 0) {
        console.error('GL spec section 6.4 error: vertex attribute data offset of ' + offset + ' bytes should have been a multiple of the data type size that was used: GLenum ' + dataType + ' has size of ' + sizeBytes + ' bytes!');
      }
      if (stride % sizeBytes != 0) {
        console.error('GL spec section 6.4 error: vertex attribute data stride of ' + stride + ' bytes should have been a multiple of the data type size that was used: GLenum ' + dataType + ' has size of ' + sizeBytes + ' bytes!');
      }
    },
#endif

#if TRACE_WEBGL_CALLS
    webGLFunctionLengths: { 'getContextAttributes': 0, 'isContextLost': 0, 'getSupportedExtensions': 0, 'createBuffer': 0, 'createFramebuffer': 0, 'createProgram': 0, 'createRenderbuffer': 0, 'createTexture': 0, 'finish': 0, 'flush': 0, 'getError': 0, 'createVertexArray': 0, 'createQuery': 0, 'createSampler': 0, 'createTransformFeedback': 0, 'endTransformFeedback': 0, 'pauseTransformFeedback': 0, 'resumeTransformFeedback': 0, 'commit': 0,
      'getExtension': 1, 'activeTexture': 1, 'blendEquation': 1, 'checkFramebufferStatus': 1, 'clear': 1, 'clearDepth': 1, 'clearStencil': 1, 'compileShader': 1, 'createShader': 1, 'cullFace': 1, 'deleteBuffer': 1, 'deleteFramebuffer': 1, 'deleteProgram': 1, 'deleteRenderbuffer': 1, 'deleteShader': 1, 'deleteTexture': 1, 'depthFunc': 1, 'depthMask': 1, 'disable': 1, 'disableVertexAttribArray': 1, 'enable': 1, 'enableVertexAttribArray': 1, 'frontFace': 1, 'generateMipmap': 1, 'getAttachedShaders': 1, 'getParameter': 1, 'getProgramInfoLog': 1, 'getShaderInfoLog': 1, 'getShaderSource': 1, 'isBuffer': 1, 'isEnabled': 1, 'isFramebuffer': 1, 'isProgram': 1, 'isRenderbuffer': 1, 'isShader': 1, 'isTexture': 1, 'lineWidth': 1, 'linkProgram': 1, 'stencilMask': 1, 'useProgram': 1, 'validateProgram': 1, 'deleteQuery': 1, 'isQuery': 1, 'deleteVertexArray': 1, 'bindVertexArray': 1, 'isVertexArray': 1, 'drawBuffers': 1, 'readBuffer': 1, 'endQuery': 1, 'deleteSampler': 1, 'isSampler': 1, 'isSync': 1, 'deleteSync': 1, 'deleteTransformFeedback': 1, 'isTransformFeedback': 1, 'beginTransformFeedback': 1,
      'attachShader': 2, 'bindBuffer': 2, 'bindFramebuffer': 2, 'bindRenderbuffer': 2, 'bindTexture': 2, 'blendEquationSeparate': 2, 'blendFunc': 2, 'depthRange': 2, 'detachShader': 2, 'getActiveAttrib': 2, 'getActiveUniform': 2, 'getAttribLocation': 2, 'getBufferParameter': 2, 'getProgramParameter': 2, 'getRenderbufferParameter': 2, 'getShaderParameter': 2, 'getShaderPrecisionFormat': 2, 'getTexParameter': 2, 'getUniform': 2, 'getUniformLocation': 2, 'getVertexAttrib': 2, 'getVertexAttribOffset': 2, 'hint': 2, 'pixelStorei': 2, 'polygonOffset': 2, 'sampleCoverage': 2, 'shaderSource': 2, 'stencilMaskSeparate': 2, 'uniform1f': 2, 'uniform1fv': 2, 'uniform1i': 2, 'uniform1iv': 2, 'uniform2fv': 2, 'uniform2iv': 2, 'uniform3fv': 2, 'uniform3iv': 2, 'uniform4fv': 2, 'uniform4iv': 2, 'vertexAttrib1f': 2, 'vertexAttrib1fv': 2, 'vertexAttrib2fv': 2, 'vertexAttrib3fv': 2, 'vertexAttrib4fv': 2, 'vertexAttribDivisor': 2, 'beginQuery': 2, 'invalidateFramebuffer': 2, 'getFragDataLocation': 2, 'uniform1ui': 2, 'uniform1uiv': 2, 'uniform2uiv': 2, 'uniform3uiv': 2, 'uniform4uiv': 2, 'vertexAttribI4iv': 2, 'vertexAttribI4uiv': 2, 'getQuery': 2, 'getQueryParameter': 2, 'bindSampler': 2, 'getSamplerParameter': 2, 'fenceSync': 2, 'getSyncParameter': 2, 'bindTransformFeedback': 2, 'getTransformFeedbackVarying': 2, 'getIndexedParameter': 2, 'getUniformIndices': 2, 'getUniformBlockIndex': 2, 'getActiveUniformBlockName': 2,
      'bindAttribLocation': 3, 'bufferData': 3, 'bufferSubData': 3, 'drawArrays': 3, 'getFramebufferAttachmentParameter': 3, 'stencilFunc': 3, 'stencilOp': 3, 'texParameterf': 3, 'texParameteri': 3, 'uniform2f': 3, 'uniform2i': 3, 'uniformMatrix2fv': 3, 'uniformMatrix3fv': 3, 'uniformMatrix4fv': 3, 'vertexAttrib2f': 3, 'getBufferSubData': 3, 'getInternalformatParameter': 3, 'uniform2ui': 3, 'uniformMatrix2x3fv': 3, 'uniformMatrix3x2fv': 3, 'uniformMatrix2x4fv': 3, 'uniformMatrix4x2fv': 3, 'uniformMatrix3x4fv': 3, 'uniformMatrix4x3fv': 3, 'clearBufferiv': 3, 'clearBufferuiv': 3, 'clearBufferfv': 3, 'samplerParameteri': 3, 'samplerParameterf': 3, 'clientWaitSync': 3, 'waitSync': 3, 'transformFeedbackVaryings': 3, 'bindBufferBase': 3, 'getActiveUniforms': 3, 'getActiveUniformBlockParameter': 3, 'uniformBlockBinding': 3,
      'blendColor': 4, 'blendFuncSeparate': 4, 'clearColor': 4, 'colorMask': 4, 'drawElements': 4, 'framebufferRenderbuffer': 4, 'renderbufferStorage': 4, 'scissor': 4, 'stencilFuncSeparate': 4, 'stencilOpSeparate': 4, 'uniform3f': 4, 'uniform3i': 4, 'vertexAttrib3f': 4, 'viewport': 4, 'drawArraysInstanced': 4, 'uniform3ui': 4, 'clearBufferfi': 4,
      'framebufferTexture2D': 5, 'uniform4f': 5, 'uniform4i': 5, 'vertexAttrib4f': 5, 'drawElementsInstanced': 5, 'copyBufferSubData': 5, 'framebufferTextureLayer': 5, 'renderbufferStorageMultisample': 5, 'texStorage2D': 5, 'uniform4ui': 5, 'vertexAttribI4i': 5, 'vertexAttribI4ui': 5, 'vertexAttribIPointer': 5, 'bindBufferRange': 5,
      'texImage2D': 6, 'vertexAttribPointer': 6, 'invalidateSubFramebuffer': 6, 'texStorage3D': 6, 'drawRangeElements': 6,
      'compressedTexImage2D': 7, 'readPixels': 7, 'texSubImage2D': 7,
      'compressedTexSubImage2D': 8, 'copyTexImage2D': 8, 'copyTexSubImage2D': 8, 'compressedTexImage3D': 8,
      'copyTexSubImage3D': 9,
      'blitFramebuffer': 10, 'texImage3D': 10, 'compressedTexSubImage3D': 10,
      'texSubImage3D': 11 },

    hookWebGLFunction: function(f, glCtx) {
      var realf = 'real_' + f;
      glCtx[realf] = glCtx[f];
      var numArgs = GL.webGLFunctionLengths[f]; // On Firefox & Chrome, could do "glCtx[realf].length", but that doesn't work on Edge, which always reports 0.
      if (numArgs === undefined) throw 'Unexpected WebGL function ' + f;
      var contextHandle = glCtx.canvas.GLctxObject.handle;
      var threadId = (typeof _pthread_self !== 'undefined') ? _pthread_self : function() { return 1; };
      // Accessing 'arguments' is super slow, so to avoid overhead, statically reason the number of arguments.
      switch(numArgs) {
        case 0: glCtx[f] = function webgl_0() { var ret = glCtx[realf](); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '() -> ' + ret); return ret; }; break;
        case 1: glCtx[f] = function webgl_1(a1) { var ret = glCtx[realf](a1); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+') -> ' + ret); return ret; }; break;
        case 2: glCtx[f] = function webgl_2(a1, a2) { var ret = glCtx[realf](a1, a2); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +') -> ' + ret); return ret; }; break;
        case 3: glCtx[f] = function webgl_3(a1, a2, a3) { var ret = glCtx[realf](a1, a2, a3); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +') -> ' + ret); return ret; }; break;
        case 4: glCtx[f] = function webgl_4(a1, a2, a3, a4) { var ret = glCtx[realf](a1, a2, a3, a4); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +') -> ' + ret); return ret; }; break;
        case 5: glCtx[f] = function webgl_5(a1, a2, a3, a4, a5) { var ret = glCtx[realf](a1, a2, a3, a4, a5); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +') -> ' + ret); return ret; }; break;
        case 6: glCtx[f] = function webgl_6(a1, a2, a3, a4, a5, a6) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +') -> ' + ret); return ret; }; break;
        case 7: glCtx[f] = function webgl_7(a1, a2, a3, a4, a5, a6, a7) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +') -> ' + ret); return ret; }; break;
        case 8: glCtx[f] = function webgl_8(a1, a2, a3, a4, a5, a6, a7, a8) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +', ' + a8 +') -> ' + ret); return ret; }; break;
        case 9: glCtx[f] = function webgl_9(a1, a2, a3, a4, a5, a6, a7, a8, a9) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +', ' + a8 +', ' + a9 +') -> ' + ret); return ret; }; break;
        case 10: glCtx[f] = function webgl_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +', ' + a8 +', ' + a9 +', ' + a10 +') -> ' + ret); return ret; }; break;
        case 11: glCtx[f] = function webgl_11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) { var ret = glCtx[realf](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11); console.error('[Thread ' + threadId() + ', GL ctx: ' + contextHandle + ']: ' + f + '('+a1+', ' + a2 +', ' + a3 +', ' + a4 +', ' + a5 +', ' + a6 +', ' + a7 +', ' + a8 +', ' + a9 +', ' + a10 +', ' + a11 +') -> ' + ret); return ret; }; break;
        default: throw 'hookWebGL failed! Unexpected length ' + glCtx[realf].length;
      }
    },

    hookWebGL: function(glCtx) {
      if (!glCtx) glCtx = this.detectWebGLContext();
      if (!glCtx) return;
      if (!((typeof WebGLRenderingContext !== 'undefined' && glCtx instanceof WebGLRenderingContext)
       || (typeof WebGL2RenderingContext !== 'undefined' && glCtx instanceof WebGL2RenderingContext))) {
        return;
      }

      if (glCtx.webGlTracerAlreadyHooked) return;
      glCtx.webGlTracerAlreadyHooked = true;

      // Hot GL functions are ones that you'd expect to find during render loops (render calls, dynamic resource uploads), cold GL functions are load time functions (shader compilation, texture/mesh creation)
      // Distinguishing between these two allows pinpointing locations of troublesome GL usage that might cause performance issues.
      for(var f in glCtx) {
        if (typeof glCtx[f] !== 'function' || f.indexOf('real_') == 0) continue;
        this.hookWebGLFunction(f, glCtx);
      }
      // The above injection won't work for texImage2D and texSubImage2D, which have multiple overloads.
      glCtx['texImage2D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
        var ret = (a7 !== undefined) ? glCtx['real_texImage2D'](a1, a2, a3, a4, a5, a6, a7, a8, a9) : glCtx['real_texImage2D'](a1, a2, a3, a4, a5, a6);
        return ret;
      };
      glCtx['texSubImage2D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9) {
        var ret = (a8 !== undefined) ? glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7, a8, a9) : glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7);
        return ret;
      };
      glCtx['texSubImage3D'] = function(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) {
        var ret = (a9 !== undefined) ? glCtx['real_texSubImage3D'](a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) : glCtx['real_texSubImage2D'](a1, a2, a3, a4, a5, a6, a7, a8);
        return ret;
      };
    },
#endif
    // Returns the context handle to the new context.
    createContext: function(canvas, webGLContextAttributes) {
      if (typeof webGLContextAttributes['majorVersion'] === 'undefined' && typeof webGLContextAttributes['minorVersion'] === 'undefined') {
#if USE_WEBGL2
        // If caller did not specify a context, initialize the best one that is possibly available.
        // To explicitly create a WebGL 1 or a WebGL 2 context, call this function with a specific
        // majorVersion set.
        if (typeof WebGL2RenderingContext !== 'undefined') webGLContextAttributes['majorVersion'] = 2;
        else webGLContextAttributes['majorVersion'] = 1;
#else
        webGLContextAttributes['majorVersion'] = 1;
#endif
        webGLContextAttributes['minorVersion'] = 0;
      }

#if OFFSCREEN_FRAMEBUFFER
      // In proxied operation mode, rAF()/setTimeout() functions do not delimit frame boundaries, so can't have WebGL implementation
      // try to detect when it's ok to discard contents of the rendered backbuffer.
      if (webGLContextAttributes['renderViaOffscreenBackBuffer']) webGLContextAttributes['preserveDrawingBuffer'] = true;
#endif

#if GL_TESTING
      webGLContextAttributes['preserveDrawingBuffer'] = true;
#endif

      var ctx;
      var errorInfo = '?';
      function onContextCreationError(event) {
        errorInfo = event.statusMessage || errorInfo;
      }
      // XX localmod: Always utilize high-performance GPU for Unity builds
      webGLContextAttributes['powerPreference'] = 'high-performance';
      try {
        canvas.addEventListener('webglcontextcreationerror', onContextCreationError, false);
        try {
#if GL_PREINITIALIZED_CONTEXT
          // If WebGL context has already been preinitialized for the page on the JS side, reuse that context instead. This is useful for example when
          // the main page precompiles shaders for the application, in which case the WebGL context is created already before any Emscripten compiled
          // code has been downloaded.
          if (Module['preinitializedWebGLContext']) {
            ctx = Module['preinitializedWebGLContext'];
            webGLContextAttributes['majorVersion'] = (typeof WebGL2RenderingContext !== 'undefined' && ctx instanceof WebGL2RenderingContext) ? 2 : 1;
          } else
#endif
          if (webGLContextAttributes['majorVersion'] == 1 && webGLContextAttributes['minorVersion'] == 0) {
            ctx = canvas.getContext("webgl", webGLContextAttributes) || canvas.getContext("experimental-webgl", webGLContextAttributes);
          } else if (webGLContextAttributes['majorVersion'] == 2 && webGLContextAttributes['minorVersion'] == 0) {
            ctx = canvas.getContext("webgl2", webGLContextAttributes);
          } else {
            throw 'Unsupported WebGL context version ' + majorVersion + '.' + minorVersion + '!'
          }
        } finally {
          canvas.removeEventListener('webglcontextcreationerror', onContextCreationError, false);
        }
        if (!ctx) throw ':(';
      } catch (e) {
        out('Could not create canvas: ' + [errorInfo, e, JSON.stringify(webGLContextAttributes)]);
        return 0;
      }

      if (!ctx) return 0;
      var context = GL.registerContext(ctx, webGLContextAttributes);
#if TRACE_WEBGL_CALLS
      GL.hookWebGL(ctx);
#endif
      return context;
    },

#if OFFSCREEN_FRAMEBUFFER
    enableOffscreenFramebufferAttributes: function(webGLContextAttributes) {
      webGLContextAttributes.renderViaOffscreenBackBuffer = true;
      webGLContextAttributes.preserveDrawingBuffer = true;
    },

    // If WebGL is being proxied from a pthread to the main thread, we can't directly render to the WebGL default back buffer
    // because of WebGL's implicit swap behavior. Therefore in such modes, create an offscreen render target surface to
    // which rendering is performed to, and finally flipped to the main screen.
    createOffscreenFramebuffer: function(context) {
      var gl = context.GLctx;

      // Create FBO
      var fbo = gl.createFramebuffer();
      gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
      context.defaultFbo = fbo;

      // Create render targets to the FBO
      context.defaultColorTarget = gl.createTexture();
      context.defaultDepthTarget = gl.createRenderbuffer();
      GL.resizeOffscreenFramebuffer(context); // Size them up correctly (use the same mechanism when resizing on demand)

      gl.bindTexture(gl.TEXTURE_2D, context.defaultColorTarget);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
      gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
      gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.canvas.width, gl.canvas.height, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
      gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, context.defaultColorTarget, 0);
      gl.bindTexture(gl.TEXTURE_2D, null);

      // Create depth render target to the FBO
      var depthTarget = gl.createRenderbuffer();
      gl.bindRenderbuffer(gl.RENDERBUFFER, context.defaultDepthTarget);
      gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, gl.canvas.width, gl.canvas.height);
      gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, context.defaultDepthTarget);
      gl.bindRenderbuffer(gl.RENDERBUFFER, null);

      // Create blitter
      var vertices = [
        -1, -1,
        -1,  1,
         1, -1,
         1,  1
      ];
      var vb = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, vb);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
      gl.bindBuffer(gl.ARRAY_BUFFER, null);
      context.blitVB = vb;

      var vsCode =
        'attribute vec2 pos;' +
        'varying lowp vec2 tex;' +
        'void main() { tex = pos * 0.5 + vec2(0.5,0.5); gl_Position = vec4(pos, 0.0, 1.0); }';
      var vs = gl.createShader(gl.VERTEX_SHADER);
      gl.shaderSource(vs, vsCode);
      gl.compileShader(vs);

      var fsCode =
        'varying lowp vec2 tex;' +
        'uniform sampler2D sampler;' +
        'void main() { gl_FragColor = texture2D(sampler, tex); }';
      var fs = gl.createShader(gl.FRAGMENT_SHADER);
      gl.shaderSource(fs, fsCode);
      gl.compileShader(fs);

      var blitProgram = gl.createProgram();
      gl.attachShader(blitProgram, vs);
      gl.attachShader(blitProgram, fs);
      gl.linkProgram(blitProgram);
      context.blitProgram = blitProgram;
      context.blitPosLoc = gl.getAttribLocation(blitProgram, "pos");
      gl.useProgram(blitProgram);
      gl.uniform1i(gl.getUniformLocation(blitProgram, "sampler"), 0);
      gl.useProgram(null);
    },

    resizeOffscreenFramebuffer: function(context) {
      var gl = context.GLctx;

      // Resize color buffer
      if (context.defaultColorTarget) {
        var prevTextureBinding = gl.getParameter(gl.TEXTURE_BINDING_2D);
        gl.bindTexture(gl.TEXTURE_2D, context.defaultColorTarget);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.drawingBufferWidth, gl.drawingBufferHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
        gl.bindTexture(gl.TEXTURE_2D, prevTextureBinding);
      }

      // Resize depth buffer
      if (context.defaultDepthTarget) {
        var prevRenderBufferBinding = gl.getParameter(gl.RENDERBUFFER_BINDING);
        gl.bindRenderbuffer(gl.RENDERBUFFER, context.defaultDepthTarget);
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, gl.drawingBufferWidth, gl.drawingBufferHeight); // TODO: Read context creation parameters for what type of depth and stencil to use
        gl.bindRenderbuffer(gl.RENDERBUFFER, prevRenderBufferBinding);
      }
    },

    // Renders the contents of the offscreen render target onto the visible screen.
    blitOffscreenFramebuffer: function(context) {
      var gl = context.GLctx;

      var prevFbo = gl.getParameter(gl.FRAMEBUFFER_BINDING);
      gl.bindFramebuffer(gl.FRAMEBUFFER, null);

      var prevProgram = gl.getParameter(gl.CURRENT_PROGRAM);
      gl.useProgram(context.blitProgram);

      var prevVB = gl.getParameter(gl.ARRAY_BUFFER_BINDING);
      gl.bindBuffer(gl.ARRAY_BUFFER, context.blitVB);

      gl.vertexAttribPointer(context.blitPosLoc, 2, gl.FLOAT, false, 0, 0);
      gl.enableVertexAttribArray(context.blitPosLoc);

      var prevActiveTexture = gl.getParameter(gl.ACTIVE_TEXTURE);
      gl.activeTexture(gl.TEXTURE0);

      var prevTextureBinding = gl.getParameter(gl.TEXTURE_BINDING_2D);
      gl.bindTexture(gl.TEXTURE_2D, context.defaultColorTarget);

      var prevBlend = gl.getParameter(gl.BLEND);
      if (prevBlend) gl.disable(gl.BLEND);

      var prevCullFace = gl.getParameter(gl.CULL_FACE);
      if (prevCullFace) gl.disable(gl.CULL_FACE);

      var prevDepthTest = gl.getParameter(gl.DEPTH_TEST);
      if (prevDepthTest) gl.disable(gl.DEPTH_TEST);

      gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

      if (prevDepthTest) gl.enable(gl.DEPTH_TEST);
      if (prevCullFace) gl.enable(gl.CULL_FACE);
      if (prevBlend) gl.enable(gl.BLEND);

      gl.bindTexture(gl.TEXTURE_2D, prevTextureBinding);
      gl.activeTexture(prevActiveTexture);
      // prevEnableVertexAttribArray?
      // prevVertexAttribPointer?
      gl.bindBuffer(gl.ARRAY_BUFFER, prevVB);
      gl.useProgram(prevProgram);
      gl.bindFramebuffer(gl.FRAMEBUFFER, prevFbo);
    },
#endif

    registerContext: function(ctx, webGLContextAttributes) {
      var handle = _malloc(8); // Make space on the heap to store GL context attributes that need to be accessible as shared between threads.
      {{{ makeSetValue('handle', 0, 'webGLContextAttributes["explicitSwapControl"]', 'i32')}}}; // explicitSwapControl
#if USE_PTHREADS
      {{{ makeSetValue('handle', 4, '_pthread_self()', 'i32')}}}; // the thread pointer of the thread that owns the control of the context
#endif
      var context = {
        handle: handle,
        attributes: webGLContextAttributes,
        version: webGLContextAttributes['majorVersion'],
        GLctx: ctx
      };

#if USE_WEBGL2
      // BUG: Workaround Chrome WebGL 2 issue: the first shipped versions of WebGL 2 in Chrome did not actually implement the new WebGL 2 functions.
      //      Those are supported only in Chrome 58 and newer.
      function getChromeVersion() {
        var raw = navigator.userAgent.match(/Chrom(e|ium)\/([0-9]+)\./);
        return raw ? parseInt(raw[2], 10) : false;
      }
      context.supportsWebGL2EntryPoints = (context.version >= 2) && (getChromeVersion() === false || getChromeVersion() >= 58);
#endif

      // Store the created context object so that we can access the context given a canvas without having to pass the parameters again.
      if (ctx.canvas) ctx.canvas.GLctxObject = context;
      GL.contexts[handle] = context;
      if (typeof webGLContextAttributes['enableExtensionsByDefault'] === 'undefined' || webGLContextAttributes['enableExtensionsByDefault']) {
        GL.initExtensions(context);
      }

      if (webGLContextAttributes['renderViaOffscreenBackBuffer']) {
#if OFFSCREEN_FRAMEBUFFER
        GL.createOffscreenFramebuffer(context);
#else
#if GL_DEBUG
        Module.printErr('renderViaOffscreenBackBuffer=true specified in WebGL context creation attributes, pass linker flag -s OFFSCREEN_FRAMEBUFFER=1 to enable support!');
#endif
        return 0;
#endif
      }

      return handle;
    },

    makeContextCurrent: function(contextHandle) {
      // Deactivating current context?
      if (!contextHandle) {
        GLctx = Module.ctx = GL.currentContext = null;
        return true;
      }
      var context = GL.contexts[contextHandle];
      if (!context) {
#if GL_DEBUG
#if USE_PTHREADS
        console.error('GL.makeContextCurrent() failed! WebGL context ' + contextHandle + ' does not exist, or was created on another thread!');
#else
        console.error('GL.makeContextCurrent() failed! WebGL context ' + contextHandle + ' does not exist!');
#endif
#endif
        return false;
      }
      GLctx = Module.ctx = context.GLctx; // Active WebGL context object.
      GL.currentContext = context; // Active Emscripten GL layer context object.
      return true;
    },

    getContext: function(contextHandle) {
      return GL.contexts[contextHandle];
    },

    deleteContext: function(contextHandle) {
      if (!contextHandle) return;
      if (GL.currentContext === GL.contexts[contextHandle]) GL.currentContext = null;
      if (typeof JSEvents === 'object') JSEvents.removeAllHandlersOnTarget(GL.contexts[contextHandle].GLctx.canvas); // Release all JS event handlers on the DOM element that the GL context is associated with since the context is now deleted.
      if (GL.contexts[contextHandle] && GL.contexts[contextHandle].GLctx.canvas) GL.contexts[contextHandle].GLctx.canvas.GLctxObject = undefined; // Make sure the canvas object no longer refers to the context object so there are no GC surprises.
      _free(GL.contexts[contextHandle]);
      GL.contexts[contextHandle] = null;
    },

    // In WebGL, extensions must be explicitly enabled to be active, see http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.14.14
    // In GLES2, all extensions are enabled by default without additional operations. Init all extensions we need to give to GLES2 user
    // code here, so that GLES2 code can operate without changing behavior.
    initExtensions: function(context) {
      // If this function is called without a specific context object, init the extensions of the currently active context.
      if (!context) context = GL.currentContext;

      if (context.initExtensionsDone) return;
      context.initExtensionsDone = true;

      var GLctx = context.GLctx;

      context.maxVertexAttribs = GLctx.getParameter(GLctx.MAX_VERTEX_ATTRIBS);
#if FULL_ES2
      context.clientBuffers = [];
      for (var i = 0; i < context.maxVertexAttribs; i++) {
        context.clientBuffers[i] = { enabled: false, clientside: false, size: 0, type: 0, normalized: 0, stride: 0, ptr: 0 };
      }

      GL.generateTempBuffers(false, context);
#endif

      // Detect the presence of a few extensions manually, this GL interop layer itself will need to know if they exist.
#if LEGACY_GL_EMULATION
      context.compressionExt = GLctx.getExtension('WEBGL_compressed_texture_s3tc');
      context.anisotropicExt = GLctx.getExtension('EXT_texture_filter_anisotropic');
#endif

      if (context.version < 2) {
        // Extension available from Firefox 26 and Google Chrome 30
        var instancedArraysExt = GLctx.getExtension('ANGLE_instanced_arrays');
        if (instancedArraysExt) {
          GLctx['vertexAttribDivisor'] = function(index, divisor) { instancedArraysExt['vertexAttribDivisorANGLE'](index, divisor); };
          GLctx['drawArraysInstanced'] = function(mode, first, count, primcount) { instancedArraysExt['drawArraysInstancedANGLE'](mode, first, count, primcount); };
          GLctx['drawElementsInstanced'] = function(mode, count, type, indices, primcount) { instancedArraysExt['drawElementsInstancedANGLE'](mode, count, type, indices, primcount); };
        }

        // Extension available from Firefox 25 and WebKit
        var vaoExt = GLctx.getExtension('OES_vertex_array_object');
        if (vaoExt) {
          GLctx['createVertexArray'] = function() { return vaoExt['createVertexArrayOES'](); };
          GLctx['deleteVertexArray'] = function(vao) { vaoExt['deleteVertexArrayOES'](vao); };
          GLctx['bindVertexArray'] = function(vao) { vaoExt['bindVertexArrayOES'](vao); };
          GLctx['isVertexArray'] = function(vao) { return vaoExt['isVertexArrayOES'](vao); };
        }

        var drawBuffersExt = GLctx.getExtension('WEBGL_draw_buffers');
        if (drawBuffersExt) {
          GLctx['drawBuffers'] = function(n, bufs) { drawBuffersExt['drawBuffersWEBGL'](n, bufs); };
        }
      }

      GLctx.disjointTimerQueryExt = GLctx.getExtension("EXT_disjoint_timer_query");

      // These are the 'safe' feature-enabling extensions that don't add any performance impact related to e.g. debugging, and
      // should be enabled by default so that client GLES2/GL code will not need to go through extra hoops to get its stuff working.
      // As new extensions are ratified at http://www.khronos.org/registry/webgl/extensions/ , feel free to add your new extensions
      // here, as long as they don't produce a performance impact for users that might not be using those extensions.
      // E.g. debugging-related extensions should probably be off by default.
      // XX localmod: Added EXT_texture_norm16 and WEBKIT_WEBGL_compressed_texture_pvrtc to the list.
      var automaticallyEnabledExtensions = [ // Khronos ratified WebGL extensions ordered by number (no debug extensions):
                                             "OES_texture_float", "OES_texture_half_float", "OES_standard_derivatives",
                                             "OES_vertex_array_object", "WEBGL_compressed_texture_s3tc", "WEBGL_depth_texture",
                                             "OES_element_index_uint", "EXT_texture_filter_anisotropic", "EXT_frag_depth",
                                             "WEBGL_draw_buffers", "ANGLE_instanced_arrays", "OES_texture_float_linear",
                                             "OES_texture_half_float_linear", "EXT_blend_minmax", "EXT_shader_texture_lod",
                                             "EXT_texture_norm16",
                                             // Community approved WebGL extensions ordered by number:
                                             "WEBGL_compressed_texture_pvrtc", "EXT_color_buffer_half_float", "WEBGL_color_buffer_float",
                                             "EXT_sRGB", "WEBGL_compressed_texture_etc1", "EXT_disjoint_timer_query",
                                             "WEBGL_compressed_texture_etc", "WEBGL_compressed_texture_astc", "EXT_color_buffer_float",
                                             "WEBGL_compressed_texture_s3tc_srgb", "EXT_disjoint_timer_query_webgl2",
                                             "WEBKIT_WEBGL_compressed_texture_pvrtc"];

      function shouldEnableAutomatically(extension) {
        var ret = false;
        automaticallyEnabledExtensions.forEach(function(include) {
          if (extension.indexOf(include) != -1) {
            ret = true;
          }
        });
        return ret;
      }

      var exts = GLctx.getSupportedExtensions();
      if (exts && exts.length > 0) {
        GLctx.getSupportedExtensions().forEach(function(ext) {
          if (automaticallyEnabledExtensions.indexOf(ext) != -1) {
            GLctx.getExtension(ext); // Calling .getExtension enables that extension permanently, no need to store the return value to be enabled.
          }
        });
      }
    },

    // In WebGL, uniforms in a shader program are accessed through an opaque object type 'WebGLUniformLocation'.
    // In GLES2, uniforms are accessed via indices. Therefore we must generate a mapping of indices -> WebGLUniformLocations
    // to provide the client code the API that uses indices.
    // This function takes a linked GL program and generates a mapping table for the program.
    // NOTE: Populating the uniform table is performed eagerly at glLinkProgram time, so glLinkProgram should be considered
    //       to be a slow/costly function call. Calling glGetUniformLocation is relatively fast, since it is always a read-only
    //       lookup to the table populated in this function call.
    populateUniformTable: function(program) {
#if GL_ASSERTIONS
      GL.validateGLObjectID(GL.programs, program, 'populateUniformTable', 'program');
#endif
      var p = GL.programs[program];
      GL.programInfos[program] = {
        uniforms: {},
        maxUniformLength: 0, // This is eagerly computed below, since we already enumerate all uniforms anyway.
        maxAttributeLength: -1, // This is lazily computed and cached, computed when/if first asked, "-1" meaning not computed yet.
        maxUniformBlockNameLength: -1 // Lazily computed as well
      };

      var ptable = GL.programInfos[program];
      var utable = ptable.uniforms;
      // A program's uniform table maps the string name of an uniform to an integer location of that uniform.
      // The global GL.uniforms map maps integer locations to WebGLUniformLocations.
      var numUniforms = GLctx.getProgramParameter(p, GLctx.ACTIVE_UNIFORMS);
      for (var i = 0; i < numUniforms; ++i) {
        var u = GLctx.getActiveUniform(p, i);

        var name = u.name;
        ptable.maxUniformLength = Math.max(ptable.maxUniformLength, name.length+1);

        // Strip off any trailing array specifier we might have got, e.g. "[0]".
        if (name.indexOf(']', name.length-1) !== -1) {
          var ls = name.lastIndexOf('[');
          name = name.slice(0, ls);
        }

        // Optimize memory usage slightly: If we have an array of uniforms, e.g. 'vec3 colors[3];', then
        // only store the string 'colors' in utable, and 'colors[0]', 'colors[1]' and 'colors[2]' will be parsed as 'colors'+i.
        // Note that for the GL.uniforms table, we still need to fetch the all WebGLUniformLocations for all the indices.
        var loc = GLctx.getUniformLocation(p, name);
        if (loc != null)
        {
          var id = GL.getNewId(GL.uniforms);
          utable[name] = [u.size, id];
          GL.uniforms[id] = loc;

          for (var j = 1; j < u.size; ++j) {
            var n = name + '['+j+']';
            loc = GLctx.getUniformLocation(p, n);
            id = GL.getNewId(GL.uniforms);

            GL.uniforms[id] = loc;
          }
        }
      }
    }
  },
};

mergeInto(LibraryManager.library, LibraryGL);
