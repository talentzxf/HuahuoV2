import {ContextMenu} from "./ContextMenu";
import {CustomElement} from "./CustomComponent";
import {Logger} from "./Logger";
import {Vector2} from "./Math/Vector2";
import {EventParam} from "./EventBus/EventBus";
import {EventEmitter} from "./EventBus/EventEmitter";
import {getFullEventName, splitFullEventName} from "./EventBus/EventBus";
import {radToDeg, degToRad} from "./Math/MathFunctions";
import {CustomFieldContentXGenerator} from "./Properties/PropertyConfig";

import {
    pointsNear,
    pointsNearHorizontal,
    pointsNearVertical,
    relaxRectangle,
    getMimeTypeFromDataURI,
    dataURItoBlob,
    getFileNameFromGZip,
    getMethodsAndVariables,
    getParameterNameAtIdx
} from "./Utils";
import {PropertySheet, Property, PropertyType} from "./Properties/PropertySheet"
import {I18nHandler} from "./translation/I18nHandler";
import "./i18ninitializer"
import {HHToast} from "./Toast/Toast";
import {eventBus, GraphEvent} from "./EventBus/EventBus";
import {mirrorPoint} from "./Math/MathFunctions";
import {IsValidWrappedObject, GetObjPtr} from "./WrappedObjectUtils"
import {
    CustomFieldConfig,
    CustomFieldContentDivGenerator,
    ShapeArrayProperty,
    StringProperty
} from "./Properties/PropertyConfig";
import {capitalizeFirstLetter} from "./Properties/Utils";

if (!window.i18n) {
    window.i18n = new I18nHandler()
}

let i18n = window.i18n

export {
    i18n, ContextMenu, CustomElement, Logger, Vector2, pointsNear, pointsNearVertical, pointsNearHorizontal,
    relaxRectangle, PropertySheet, Property, PropertyType, getMimeTypeFromDataURI,
    dataURItoBlob, HHToast, getFileNameFromGZip, eventBus, mirrorPoint,
    IsValidWrappedObject, GetObjPtr, getMethodsAndVariables,
    CustomFieldContentDivGenerator, CustomFieldConfig,
    GraphEvent, EventParam, EventEmitter, getParameterNameAtIdx,
    getFullEventName, splitFullEventName, ShapeArrayProperty,
    capitalizeFirstLetter, StringProperty, radToDeg, degToRad, CustomFieldContentXGenerator
}

