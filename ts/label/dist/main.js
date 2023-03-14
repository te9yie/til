/*
 * ATTENTION: The "eval" devtool has been used (maybe by default in mode: "development").
 * This devtool is neither made for production nor for readable output files.
 * It uses "eval()" calls to create a separate source file in the browser devtools.
 * If you are trying to read the output file, select a different devtool (https://webpack.js.org/configuration/devtool/)
 * or disable the default devtool with "devtool: false".
 * If you are looking for production-ready output files, see mode: "production" (https://webpack.js.org/configuration/mode/).
 */
/******/ (() => { // webpackBootstrap
/******/ 	"use strict";
/******/ 	var __webpack_modules__ = ({

/***/ "./src/geometry.ts":
/*!*************************!*\
  !*** ./src/geometry.ts ***!
  \*************************/
/***/ ((__unused_webpack_module, exports) => {

eval("\r\nexports.__esModule = true;\r\nexports.Size = exports.Pos = void 0;\r\nvar Pos = /** @class */ (function () {\r\n    function Pos(x, y) {\r\n        this.x = x;\r\n        this.y = y;\r\n    }\r\n    return Pos;\r\n}());\r\nexports.Pos = Pos;\r\nvar Size = /** @class */ (function () {\r\n    function Size(w, h) {\r\n        this.w = w;\r\n        this.h = h;\r\n    }\r\n    return Size;\r\n}());\r\nexports.Size = Size;\r\n\n\n//# sourceURL=webpack://canvas/./src/geometry.ts?");

/***/ }),

/***/ "./src/index.ts":
/*!**********************!*\
  !*** ./src/index.ts ***!
  \**********************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

eval("\r\nexports.__esModule = true;\r\nvar geometry_1 = __webpack_require__(/*! ./geometry */ \"./src/geometry.ts\");\r\nvar label_1 = __webpack_require__(/*! ./label */ \"./src/label.ts\");\r\ndocument.addEventListener(\"DOMContentLoaded\", function () {\r\n    var canvas = document.createElement(\"canvas\");\r\n    var ctx = canvas.getContext(\"2d\");\r\n    canvas.style.border = \"1px solid\";\r\n    ctx.font = \"12px sans-serif\";\r\n    var label = new label_1.Label(\"the quick brown fox jumps over the lazy dog\");\r\n    var s = label.update_layout(ctx, new geometry_1.Size(canvas.width, canvas.height));\r\n    var offset = new geometry_1.Pos(10, 10);\r\n    label.render(ctx, offset);\r\n    label.render(ctx, new geometry_1.Pos(offset.x, offset.y + s.h));\r\n    ctx.strokeRect(offset.x, offset.y, s.w, s.h * 2);\r\n    document.body.appendChild(canvas);\r\n});\r\n\n\n//# sourceURL=webpack://canvas/./src/index.ts?");

/***/ }),

/***/ "./src/label.ts":
/*!**********************!*\
  !*** ./src/label.ts ***!
  \**********************/
/***/ ((__unused_webpack_module, exports, __webpack_require__) => {

eval("\r\nexports.__esModule = true;\r\nexports.Label = void 0;\r\nvar geometry_1 = __webpack_require__(/*! ./geometry */ \"./src/geometry.ts\");\r\nvar Label = /** @class */ (function () {\r\n    function Label(text) {\r\n        this.text = text;\r\n    }\r\n    Label.prototype.update_layout = function (ctx, hintSize) {\r\n        var m = ctx.measureText(this.text);\r\n        return new geometry_1.Size(m.width, m.fontBoundingBoxAscent + m.fontBoundingBoxDescent);\r\n    };\r\n    Label.prototype.render = function (ctx, p) {\r\n        var m = ctx.measureText(this.text);\r\n        ctx.fillText(this.text, p.x, p.y + m.fontBoundingBoxAscent);\r\n    };\r\n    return Label;\r\n}());\r\nexports.Label = Label;\r\n\n\n//# sourceURL=webpack://canvas/./src/label.ts?");

/***/ })

/******/ 	});
/************************************************************************/
/******/ 	// The module cache
/******/ 	var __webpack_module_cache__ = {};
/******/ 	
/******/ 	// The require function
/******/ 	function __webpack_require__(moduleId) {
/******/ 		// Check if module is in cache
/******/ 		var cachedModule = __webpack_module_cache__[moduleId];
/******/ 		if (cachedModule !== undefined) {
/******/ 			return cachedModule.exports;
/******/ 		}
/******/ 		// Create a new module (and put it into the cache)
/******/ 		var module = __webpack_module_cache__[moduleId] = {
/******/ 			// no module.id needed
/******/ 			// no module.loaded needed
/******/ 			exports: {}
/******/ 		};
/******/ 	
/******/ 		// Execute the module function
/******/ 		__webpack_modules__[moduleId](module, module.exports, __webpack_require__);
/******/ 	
/******/ 		// Return the exports of the module
/******/ 		return module.exports;
/******/ 	}
/******/ 	
/************************************************************************/
/******/ 	
/******/ 	// startup
/******/ 	// Load entry module and return exports
/******/ 	// This entry module can't be inlined because the eval devtool is used.
/******/ 	var __webpack_exports__ = __webpack_require__("./src/index.ts");
/******/ 	
/******/ })()
;