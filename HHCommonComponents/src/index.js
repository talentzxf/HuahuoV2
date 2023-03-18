import {ContextMenu} from "./ContextMenu";
import {CustomElement} from "./CustomComponent";
import {Logger} from "./Logger";
import {Vector2} from "./Math/Vector2";
import {EventOut} from "./EventBus/EventBus";
import {EventParameterTypes} from "./EventBus/EventParameterTypes";

import {
    pointsNear,
    relaxRectangle,
    getMimeTypeFromDataURI,
    dataURItoBlob,
    getFileNameFromGZip,
    getMethodsAndVariables
} from "./Utils";
import {PropertySheet, Property, PropertyType} from "./Properties/PropertySheet"
import {I18nHandler} from "./translation/I18nHandler";
import "./i18ninitializer"
import {HHToast} from "./Toast/Toast";
import {eventBus, TriggerEvent} from "./EventBus/EventBus";
import {mirrorPoint} from "./Math/MathFunctions";
import {IsValidWrappedObject} from "./WrappedObjectUtils"
import {CustomFieldConfig, CustomFieldContentDivGenerator} from "./Properties/PropertyConfig";

if (!window.i18n) {
    window.i18n = new I18nHandler()
}

let i18n = window.i18n

export {
    i18n, ContextMenu, CustomElement, Logger, Vector2, pointsNear,
    relaxRectangle, PropertySheet, Property, PropertyType, getMimeTypeFromDataURI,
    dataURItoBlob, HHToast, getFileNameFromGZip, eventBus, mirrorPoint,
    IsValidWrappedObject, getMethodsAndVariables,
    CustomFieldContentDivGenerator, CustomFieldConfig,
    TriggerEvent, EventOut, EventParameterTypes
}

