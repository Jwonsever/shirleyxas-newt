/*
 * Copyright (c) 2011 Anatomical Travelogue LLC. All rights reserved.
 * (http://www.anatomicaltravel.com/research)
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met:
 *  - Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY ANATOMICAL TRAVELOGUE LLC ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ANATOMICAL TRAVELOGUE LLC 
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

var gl;
var canvas;
var volumeTexture;

var width = -1;
var height = -1;

var scaleX = 1.0;
var scaleY = 1.0;
var scaleZ = 1.0;

var raysteps = 64;
var raystepsgroup = 10;
var stepsdisplay = null;

var linearFiltering = false;
var hiqdisplay = null;

var opacity = 4.0;
var brightness = 5.0;

var rotationMatrix = null;
var distance = 7.0;

var mouseDown = false;
var mouseLastX = null;
var mouseLastY = null;

var touchDown = false;
var touchLastX = null;
var touchLastY = null;

var mouseDownRed = false;
var mouseDownGreen = false;
var mouseDownBlue = false;
var mouseDownAlpha = false;

var touchDownRed = false;
var touchDownGreen = false;
var touchDownBlue = false;
var touchDownAlpha = false;

var canvasRed;
var contextRed;
var canvasGreen;
var contextGreen;
var canvasBlue;
var contextBlue;
var canvasAlpha;
var contextAlpha;

var width2D = 512;
var height2D = 32;
var width2DScale = 2.0;
var height2DScale = 0.125;
var border2D = 5;

var transferFunction = new Uint8Array(1024);
var transferFunctionChanged = false;
var transferFunctionLastIndexRed = -1;
var transferFunctionLastIndexGreen = -1;
var transferFunctionLastIndexBlue = -1;
var transferFunctionLastIndexAlpha = -1;

function initCanvases()
{
	canvasRed = document.getElementById("canvas_red");
	if (!canvasRed.getContext)
		return;
	canvasGreen = document.getElementById("canvas_green");
	if (!canvasGreen.getContext)
		return;
	canvasBlue = document.getElementById("canvas_blue");
	if (!canvasBlue.getContext)
		return;
	canvasAlpha = document.getElementById("canvas_alpha");
	if (!canvasAlpha.getContext)
		return;

	contextRed = canvasRed.getContext("2d");
	if (!contextRed)
		return;
	contextGreen = canvasGreen.getContext("2d");
	if (!contextGreen)
		return;
	contextBlue = canvasBlue.getContext("2d");
	if (!contextBlue)
		return;
	contextAlpha = canvasAlpha.getContext("2d");
	if (!contextAlpha)
		return;

	canvasRed.onmousedown = handleMouseDownRed;
	canvasGreen.onmousedown = handleMouseDownGreen;
	canvasBlue.onmousedown = handleMouseDownBlue;
	canvasAlpha.onmousedown = handleMouseDownAlpha;

	canvasRed.onmousemove = handleMouseMoveRed;
	canvasGreen.onmousemove = handleMouseMoveGreen;
	canvasBlue.onmousemove = handleMouseMoveBlue;
	canvasAlpha.onmousemove = handleMouseMoveAlpha;

	document.onmouseup = handleMouseUp;

	canvasRed.addEventListener('touchstart', handleTouchDownRed);
	canvasGreen.addEventListener('touchstart', handleTouchDownGreen);
	canvasBlue.addEventListener('touchstart', handleTouchDownBlue);
	canvasAlpha.addEventListener('touchstart', handleTouchDownAlpha);

	canvasRed.addEventListener('touchmove', handleTouchMoveRed);
	canvasGreen.addEventListener('touchmove', handleTouchMoveGreen);
	canvasBlue.addEventListener('touchmove', handleTouchMoveBlue);
	canvasAlpha.addEventListener('touchmove', handleTouchMoveAlpha);

	document.touchend = handleTouchUp;

	clear2D();
}

function clear2D(indexOffset)
{
	if (indexOffset || indexOffset == 0)
	{
		if (indexOffset < 3)
		{
			for (i = 0; i < 256; i++)
			{
				transferFunction[i * 4 + indexOffset] = i;
			}
		}
		else if (indexOffset == 3)
		{
			for (i = 0; i < 256; i++)
			{
				transferFunction[i * 4 + indexOffset] = 64;
			}
		}
	}
	else
	{
		for (i = 0; i < 256; i++)
		{
			transferFunction[i * 4] = i;
			transferFunction[i * 4 + 1] = i;
			transferFunction[i * 4 + 2] = i;
			transferFunction[i * 4 + 3] = 64;
		}
	}

	transferFunctionChanged = true;
	draw2D();
}

function draw2D()
{
	// transfer function red
	contextRed.globalAlpha = 1.0;
	contextRed.fillStyle = "#101010";
	contextRed.fillRect(0, 0, width2D + 2 * border2D, height2D + 2 * border2D);
	var gradientX = contextRed.createLinearGradient(0, 0, width2D, 0);
	gradientX.addColorStop(0, "#000000");
	gradientX.addColorStop(1, "#ffffff");
	contextRed.fillStyle = gradientX;
	contextRed.globalAlpha = 1.0;
	contextRed.fillRect(border2D, border2D, width2D, height2D);
	var gradientY = contextRed.createLinearGradient(0, 0, 0, height2D);
	gradientY.addColorStop(0, "#ff0000");
	gradientY.addColorStop(1, "#000000");
	contextRed.globalAlpha = 0.5;
	contextRed.fillStyle = gradientY;
	contextRed.fillRect(border2D, border2D, width2D, height2D);

	contextRed.beginPath();
	contextRed.moveTo(- 0.5, border2D + height2D + 0.5);
	contextRed.lineTo(border2D + 0.5, border2D + height2D + 0.5);
	for (i = 0; i < 256; i++)
		contextRed.lineTo(border2D + Math.floor((i + 0.5) * width2DScale) + 0.5, border2D + Math.floor(height2D - transferFunction[i * 4] * height2DScale) + 0.5);
	contextRed.lineTo(border2D + 256 * width2DScale + 0.5, border2D + 0.5);
	contextRed.lineTo(2 * border2D + 256 * width2DScale + 0.5, border2D + 0.5);
	contextRed.globalAlpha = 1.0;
	contextRed.strokeStyle = "#ffffff";
	contextRed.stroke();

	// transfer function green
	contextGreen.globalAlpha = 1.0;
	contextGreen.fillStyle = "#101010";
	contextGreen.fillRect(0, 0, width2D + 2 * border2D, height2D + 2 * border2D);
	var gradientX = contextGreen.createLinearGradient(0, 0, width2D, 0);
	gradientX.addColorStop(0, "#000000");
	gradientX.addColorStop(1, "#ffffff");
	contextGreen.fillStyle = gradientX;
	contextGreen.globalAlpha = 1.0;
	contextGreen.fillRect(border2D, border2D, width2D, height2D);
	var gradientY = contextGreen.createLinearGradient(0, 0, 0, height2D);
	gradientY.addColorStop(0, "#00ff00");
	gradientY.addColorStop(1, "#000000");
	contextGreen.globalAlpha = 0.5;
	contextGreen.fillStyle = gradientY;
	contextGreen.fillRect(border2D, border2D, width2D, height2D);

	contextGreen.beginPath();
	contextGreen.moveTo(- 0.5, border2D + height2D + 0.5);
	contextGreen.lineTo(border2D + 0.5, border2D + height2D + 0.5);
	for (i = 0; i < 256; i++)
		contextGreen.lineTo(border2D + Math.floor((i + 0.5) * width2DScale) + 0.5, border2D + Math.floor(height2D - transferFunction[i * 4 + 1] * height2DScale) + 0.5);
	contextGreen.lineTo(border2D + 256 * width2DScale + 0.5, border2D + 0.5);
	contextGreen.lineTo(2 * border2D + 256 * width2DScale + 0.5, border2D + 0.5);
	contextGreen.globalAlpha = 1.0;
	contextGreen.strokeStyle = "#ffffff";
	contextGreen.stroke();

	// transfer function blue
	contextBlue.globalAlpha = 1.0;
	contextBlue.fillStyle = "#101010";
	contextBlue.fillRect(0, 0, width2D + 2 * border2D, height2D + 2 * border2D);
	var gradientX = contextBlue.createLinearGradient(0, 0, width2D, 0);
	gradientX.addColorStop(0, "#000000");
	gradientX.addColorStop(1, "#ffffff");
	contextBlue.fillStyle = gradientX;
	contextBlue.globalAlpha = 1.0;
	contextBlue.fillRect(border2D, border2D, width2D, height2D);
	var gradientY = contextBlue.createLinearGradient(0, 0, 0, height2D);
	gradientY.addColorStop(0, "#0000ff");
	gradientY.addColorStop(1, "#000000");
	contextBlue.globalAlpha = 0.5;
	contextBlue.fillStyle = gradientY;
	contextBlue.fillRect(border2D, border2D, width2D, height2D);

	contextBlue.beginPath();
	contextBlue.moveTo(- 0.5, border2D + height2D + 0.5);
	contextBlue.lineTo(border2D + 0.5, border2D + height2D + 0.5);
	for (i = 0; i < 256; i++)
		contextBlue.lineTo(border2D + Math.floor((i + 0.5) * width2DScale) + 0.5, border2D + Math.floor(height2D - transferFunction[i * 4 + 2] * height2DScale) + 0.5);
	contextBlue.lineTo(border2D + 256 * width2DScale + 0.5, border2D + 0.5);
	contextBlue.lineTo(2 * border2D + 256 * width2DScale + 0.5, border2D + 0.5);
	contextBlue.globalAlpha = 1.0;
	contextBlue.strokeStyle = "#ffffff";
	contextBlue.stroke();

	// transfer function alpha
	contextAlpha.globalAlpha = 1.0;
	contextAlpha.fillStyle = "#101010";
	contextAlpha.fillRect(0, 0, width2D + 2 * border2D, height2D + 2 * border2D);
	var gradientX = contextAlpha.createLinearGradient(0, 0, width2D, 0);
	gradientX.addColorStop(0, "#000000");
	gradientX.addColorStop(1, "#ffffff");
	contextAlpha.fillStyle = gradientX;
	contextAlpha.globalAlpha = 1.0;
	contextAlpha.fillRect(border2D, border2D, width2D, height2D);
	var gradientY = contextAlpha.createLinearGradient(0, 0, 0, height2D);
	gradientY.addColorStop(0, "#808080");
	gradientY.addColorStop(1, "#000000");
	contextBlue.globalAlpha = 0.5;
	contextAlpha.fillStyle = gradientY;
	contextAlpha.fillRect(border2D, border2D, width2D, height2D);

	contextAlpha.beginPath();
	contextAlpha.moveTo(- 0.5, border2D + height2D + 0.5);
	contextAlpha.lineTo(border2D + 0.5, border2D + height2D + 0.5);
	for (i = 0; i < 256; i++)
		contextAlpha.lineTo(border2D + Math.floor((i + 0.5) * width2DScale) + 0.5, border2D + Math.floor(height2D - transferFunction[i * 4 + 3] * height2DScale) + 0.5);
	contextAlpha.lineTo(border2D + 256 * width2DScale + 0.5, border2D + 0.5);
	contextAlpha.lineTo(2 * border2D + 256 * width2DScale + 0.5, border2D + 0.5);
	contextAlpha.globalAlpha = 1.0;
	contextAlpha.strokeStyle = "#ffffff";
	contextAlpha.stroke();
}

function handleMouseDownRed(event)
{
	mouseDownRed = true;
}

function handleMouseDownGreen(event)
{
	mouseDownGreen = true;
}

function handleMouseDownBlue(event)
{
	mouseDownBlue = true;
}

function handleMouseDownAlpha(event)
{
	mouseDownAlpha = true;
}

function handleMouseMoveRed(event)
{
	if (mouseDownRed)
		transferFunctionLastIndexRed = handleMove2D(event.pageX, event.pageY, canvasRed, 0, transferFunctionLastIndexRed);
}

function handleMouseMoveGreen(event)
{
	if (mouseDownGreen)
		transferFunctionLastIndexGreen = handleMove2D(event.pageX, event.pageY, canvasGreen, 1, transferFunctionLastIndexGreen);
}

function handleMouseMoveBlue(event)
{
	if (mouseDownBlue)
		transferFunctionLastIndexBlue = handleMove2D(event.pageX, event.pageY, canvasBlue, 2, transferFunctionLastIndexBlue);
}

function handleMouseMoveAlpha(event)
{
	if (mouseDownAlpha)
		transferFunctionLastIndexAlpha = handleMove2D(event.pageX, event.pageY, canvasAlpha, 3, transferFunctionLastIndexAlpha);
}

function handleTouchDownRed(event)
{
	if (event.targetTouches.length == 1)
	{
		touchDownRed = true;
		event.preventDefault();
	}
}

function handleTouchDownGreen(event)
{
	if (event.targetTouches.length == 1)
	{
		touchDownGreen = true;
		event.preventDefault();
	}
}

function handleTouchDownBlue(event)
{
	if (event.targetTouches.length == 1)
	{
		touchDownBlue = true;
		event.preventDefault();
	}
}

function handleTouchDownAlpha(event)
{
	if (event.targetTouches.length == 1)
	{
		touchDownAlpha = true;
		event.preventDefault();
	}
}

function handleTouchMoveRed(event)
{
	if (touchDownRed && event.targetTouches.length == 1)
	{
		transferFunctionLastIndexRed = handleMove2D(event.targetTouches[0].pageX, event.targetTouches[0].pageY, canvasRed, 0, transferFunctionLastIndexRed);
		event.preventDefault();
	}
}

function handleTouchMoveGreen(event)
{
	if (touchDownGreen && event.targetTouches.length == 1)
	{
		transferFunctionLastIndexGreen = handleMove2D(event.targetTouches[0].pageX, event.targetTouches[0].pageY, canvasGreen, 1, transferFunctionLastIndexGreen);
		event.preventDefault();
	}
}

function handleTouchMoveBlue(event)
{
	if (touchDownBlue && event.targetTouches.length == 1)
	{
		transferFunctionLastIndexBlue = handleMove2D(event.targetTouches[0].pageX, event.targetTouches[0].pageY, canvasBlue, 2, transferFunctionLastIndexBlue);
		event.preventDefault();
	}
}

function handleTouchMoveAlpha(event)
{
	if (touchDownAlpha && event.targetTouches.length == 1)
	{
		transferFunctionLastIndexAlpha = handleMove2D(event.targetTouches[0].pageX, event.targetTouches[0].pageY, canvasAlpha, 3, transferFunctionLastIndexAlpha);
		event.preventDefault();
	}
}

function handleMove2D(posX, posY, canvas, indexOffset, lastIndex)
{
	var x = posX - canvas.offsetLeft;
	var y = posY - canvas.offsetTop;

	var index = Math.min(Math.max(Math.floor((x - border2D) / width2DScale), 0), 255);
	if (lastIndex < 0)
	{
		transferFunction[index * 4 + indexOffset] = Math.min(Math.max(Math.ceil((height2D - y + border2D) / height2DScale), 0), 255);
	}
	else if (lastIndex < index)
	{
		for (var i = lastIndex + 1; i <= index; i++)
			transferFunction[i * 4 + indexOffset] = Math.min(Math.max(Math.ceil((height2D - y + border2D) / height2DScale), 0), 255);
	}
	else
	{
		for (var i = lastIndex - 1; i >= index; i--)
			transferFunction[i * 4 + indexOffset] = Math.min(Math.max(Math.ceil((height2D - y + border2D) / height2DScale), 0), 255);
	}

	transferFunctionChanged = true;
	draw2D();

	return index;
}

function start(texFile, texWidth, texHeight, texDepth, texCols, texRows)
{
	if (volumeTexture)
		return;

	gl = initWebGL("webgl_canvas", "vshader", "fshader", [ "vPosition" ], [ 0, 0, 0, 1 ], 10000);
	if (!gl)
		return;

	canvas = document.getElementById("webgl_canvas");

	volumeTexture = initVolumeTexture(gl, canvas, texFile, texWidth, texHeight, texDepth, texCols, texRows, raysteps, raystepsgroup, true, true);
	if (!volumeTexture)
	{
		alert("Your graphics card is not able to process the volume rendering algorithm.");
		return;
	}

	gl.modelViewMatrix = new J3DIMatrix4();
	gl.perspectiveMatrix = new J3DIMatrix4();
	gl.modelViewProjMatrix = new J3DIMatrix4();

	rotationMatrix = new J3DIMatrix4();

	canvas.onmousedown = handleMouseDown;
	document.onmouseup = handleMouseUp;
	document.onmousemove = handleMouseMove;

	canvas.addEventListener('touchstart', handleTouchDown);
	document.addEventListener('touchmove', handleTouchMove);
	document.addEventListener('touchend', handleTouchUp);

	setVolumeTextureOpacity(gl, volumeTexture, opacity);
	setVolumeTextureBrightness(gl, volumeTexture, brightness);

	hiqdisplay = document.getElementById("hiq");
	if (hiqdisplay) hiqdisplay.innerHTML = linearFiltering ? 'on' : 'off';
	stepsdisplay = document.getElementById("steps");
	if (stepsdisplay) stepsdisplay.innerHTML = 'Steps: ' + raysteps;
	framerate = new Framerate("framerate");

	var f = function() {
		window.requestAnimFrame(f, canvas);
		
		resize(gl);
		
		gl.modelViewMatrix.makeIdentity();
		gl.modelViewMatrix.multiply(rotationMatrix);
		gl.modelViewMatrix.scale(2.0 * scaleX, 2.0 * scaleY, 2.0 * scaleZ);
		gl.modelViewMatrix.translate(-0.5, -0.5, -0.5);
		gl.modelViewProjMatrix.makeIdentity();
		gl.modelViewProjMatrix.load(gl.perspectiveMatrix);
		gl.modelViewProjMatrix.multiply(gl.modelViewMatrix);
		
		if (transferFunctionChanged)
		{
			drawVolumeTexture(gl, volumeTexture, gl.modelViewProjMatrix.getAsFloat32Array(), linearFiltering, transferFunction);
			transferFunctionChanged = false;
		}
		else
		{
			drawVolumeTexture(gl, volumeTexture, gl.modelViewProjMatrix.getAsFloat32Array(), linearFiltering);
		}

		framerate.snapshot();
	};
	f();
}

function resize(gl, update)
{
	if (!canvas || canvas.width == width && canvas.height == height && !update)
		return;

	width = canvas.width;
	height = canvas.height;

	gl.viewport(0, 0, width, height);
	gl.perspectiveMatrix.makeIdentity();
	gl.perspectiveMatrix.perspective(20, width / height, 1, 100);
	gl.perspectiveMatrix.lookat(0, 0, distance, 0, 0, 0, 0, 1, 0);
}

function handleMouseDown(event)
{
	mouseDown = true;
	mouseLastX = event.pageX - canvas.offsetLeft;
	mouseLastY = event.pageY - canvas.offsetTop;
}

function handleMouseUp(event)
{
	mouseDown = false;
	mouseDownRed = false;
	mouseDownGreen = false;
	mouseDownBlue = false;
	mouseDownAlpha = false;
	transferFunctionLastIndexRed = -1;
	transferFunctionLastIndexGreen = -1;
	transferFunctionLastIndexBlue = -1;
	transferFunctionLastIndexAlpha = -1;
}

function handleMouseMove(event)
{
	if (mouseDown && rotationMatrix)
	{
		var mouseX = event.pageX - canvas.offsetLeft;
		var mouseY = event.pageY - canvas.offsetTop;
		var rotationTemp = new J3DIMatrix4();
		rotationTemp.makeIdentity();
		rotationTemp.rotate((mouseX - mouseLastX) / 5, 0, 1, 0);
		rotationTemp.rotate((mouseY - mouseLastY) / 5, 1, 0, 0);
		rotationTemp.multiply(rotationMatrix);
		rotationMatrix.load(rotationTemp);
		mouseLastX = mouseX
		mouseLastY = mouseY;
	}
}

function handleTouchDown(event)
{
	if (event.targetTouches.length == 1)
	{
		touchDown = true;
		touchLastX = event.targetTouches[0].pageX - canvas.offsetLeft;
		touchLastY = event.targetTouches[0].pageY - canvas.offsetTop;
		event.preventDefault();
	}
}

function handleTouchUp(event)
{
	touchDown = false;
	touchDownRed = false;
	touchDownGreen = false;
	touchDownBlue = false;
	touchDownAlpha = false;
	transferFunctionLastIndexRed = -1;
	transferFunctionLastIndexGreen = -1;
	transferFunctionLastIndexBlue = -1;
	transferFunctionLastIndexAlpha = -1;
}

function handleTouchMove(event)
{
	if (touchDown && rotationMatrix && event.touches.length == 1)
	{
		var touchX = event.touches[0].pageX - canvas.offsetLeft;
		var touchY = event.touches[0].pageY - canvas.offsetTop;
		var rotationTemp = new J3DIMatrix4();
		rotationTemp.makeIdentity();
		rotationTemp.rotate((touchX - touchLastX) / 5, 0, 1, 0);
		rotationTemp.rotate((touchY - touchLastY) / 5, 1, 0, 0);
		rotationTemp.multiply(rotationMatrix);
		rotationMatrix.load(rotationTemp);
		touchLastX = touchX
		touchLastY = touchY;
		event.preventDefault();
	}
}

function loadHead()
{
	scaleX = 1.00;
	scaleY = 1.00;
	scaleZ = 1.00;

	if (!volumeTexture)
		start("montage.jpg", 256, 256, 216, 16, 14);
	else
		setVolumeTexture(gl, volumeTexture, "montage.jpg", 256, 256, 216, 16, 14);

	if (rotationMatrix)
	{
		rotationMatrix.makeIdentity();
		rotationMatrix.rotate(90.0, 0.0, 1.0, 0.0);
		rotationMatrix.rotate(90.0, 1.0, 0.0, 0.0);
	}
}

function loadChameleon()
{
        loadCustomMontage("http://portal.nersc.gov/project/als/renders/20130608_134036_Nagaoka_4_8_V1-gridrec-20130608_211128.h5/montage.jpg", 302, 302, 38, 7, 6);
}

function loadCustomMontage(dataset,npixelsx,npixelsy,npixelsz,nx,ny)
{
	scaleX = 1.0;
	scaleY = parseFloat(npixelsy)/parseFloat(npixelsx);
	scaleZ = parseFloat(npixelsz)/parseFloat(npixelsx);

        montagelocation = "http://portal.nersc.gov/project/als/renders/"+dataset+"/montage.jpg";

        //alert(" "+scaleX+" "+scaleY+" "+scaleZ+" "+nx+" "+ny)

	if (!volumeTexture)
		start(montagelocation, npixelsx, npixelsy, npixelsz, nx, ny);
	else
		setVolumeTexture(gl, volumeTexture, montagelocation, npixelsx, npixelsy, npixelsz, nx, ny);

	if (rotationMatrix)
	{
		rotationMatrix.makeIdentity();
		//rotationMatrix.rotate(90.0, 0.0, 1.0, 0.0);
	}
}

function decreaseOpacity()
{
	opacity = Math.max(1.0, opacity - 1.0);
	setVolumeTextureOpacity(gl, volumeTexture, opacity);
}

function increaseOpacity()
{
	opacity += 1.0;
	setVolumeTextureOpacity(gl, volumeTexture, opacity);
}

function decreaseBrightness()
{
	brightness = Math.max(1.0, brightness - 1.0);
	setVolumeTextureBrightness(gl, volumeTexture, brightness);
}

function increaseBrightness()
{
	brightness = Math.min(32.0, brightness + 1.0);
	setVolumeTextureBrightness(gl, volumeTexture, brightness);
}

function decreaseSize()
{
	distance = Math.min(16.0, distance + 1.0);
	resize(gl, true);
}

function increaseSize()
{
	distance = Math.max(4.0, distance - 1.0);
	resize(gl, true);
}

function toggleLinearFiltering()
{
	linearFiltering = !linearFiltering;
	if (hiqdisplay)
		hiqdisplay.innerHTML = linearFiltering ? 'on' : 'off'
}

function decreaseNumberOfSteps()
{
	raysteps = Math.max(8.0, raysteps - Math.min(0.5 * raysteps, 64.0));
	if (stepsdisplay)
		stepsdisplay.innerHTML = 'Steps: ' + raysteps;
	setVolumeTextureRaySteps(gl, volumeTexture, raysteps)
}

function increaseNumberOfSteps()
{
	raysteps = Math.min(256.0, raysteps + Math.min(raysteps, 64.0));
	if (stepsdisplay)
		stepsdisplay.innerHTML = 'Steps: ' + raysteps;
	setVolumeTextureRaySteps(gl, volumeTexture, raysteps);
}