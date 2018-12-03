#include "Mouse.h"
#pragma comment(lib, "user32.lib")

// init routine for mouse type INPUT structure
#define INIT_INPUT(var) \
	    INPUT var; \
	    ZeroMemory(&var, sizeof(INPUT)); \
	    var.type = INPUT_MOUSE;

//  A HIGH LEVEL MOUSE INPUT CLASS
//  Requires window to be in focus/foreground
namespace MouseInput
{
	// translates button to its constant
	long Mouse::translateMouseButton(MouseButton button) {
		return buttonTable[button];
	}

	void Mouse::getMousePosition(int& xPos, int& yPos)	{
		// create POINT object named "cursorPos" for "cursor position"
		POINT cursorPos;
		// call the function "GetCursorPos" and pass it the a pointer to our POINT cursorPos
		GetCursorPos(&cursorPos);
		// read the x property from cursorPos and assign it to the integer variable xPos (a pointer, technically)
		xPos = cursorPos.x;
		// do the same thing for the y position
		yPos = cursorPos.y;
	}

	void Mouse::moveTo(int toX, int toY, bool leftDown, bool rightDown) {
		// get system information regarding screen size / resolution
		const double screenWidth = GetSystemMetrics(SM_CXSCREEN) - 1;
		const double screenHeight = GetSystemMetrics(SM_CYSCREEN) - 1;
		// scale the function inputs 'toX and 'toY' appropriately by a factor of 65535
		const double dX = toX * (65535.0f / screenWidth);
		const double dY = toY * (65535.0f / screenHeight);
		// set up INPUT Input, assign values, and move the cursor
		INIT_INPUT(input);
		// set destination coords
		input.mi.dx = static_cast<LONG>(dX);
		input.mi.dy = static_cast<LONG>(dY);
		// put up absolute move flag
		if (leftDown) {
			input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN;
		}
		else if (rightDown) {
			input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_RIGHTDOWN;
		}
		else input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

		// finally send the premade input
		SendInput(1, &input, sizeof(INPUT));
	}

	void Mouse::move(int x, int y) {
		INIT_INPUT(input);
		input.mi.dx = x;
		input.mi.dy = y;
		input.mi.dwFlags = MOUSEEVENTF_MOVE;
		SendInput(1, &input, sizeof(INPUT));
	}

	void Mouse::pressButton(MouseButton button) {
		INIT_INPUT(input);
		input.mi.dwFlags = translateMouseButton(button);
		SendInput(1, &input, sizeof(INPUT));
	}

	void Mouse::releaseButton(MouseButton button) {
		INIT_INPUT(input);
		input.mi.dwFlags = translateMouseButton(button) << 1;
		SendInput(1, &input, sizeof(INPUT));
	}

	void Mouse::wheelUp() {
		INIT_INPUT(input);
		input.mi.dwFlags = MOUSEEVENTF_WHEEL;
		input.mi.mouseData = WHEEL_DELTA;
		SendInput(1, &input, sizeof(INPUT));
	}

	void Mouse::wheelDown() {
		INIT_INPUT(input);
		input.mi.dwFlags = MOUSEEVENTF_WHEEL;
		input.mi.mouseData = -WHEEL_DELTA;
		SendInput(1, &input, sizeof(INPUT));
	}

	void Mouse::leftClick() {
		pressButton(Mouse_Left);
		releaseButton(Mouse_Left);
	}

	void Mouse::middleClick() {
		pressButton(Mouse_Middle);
		releaseButton(Mouse_Middle);
	}

	void Mouse::rightClick() {
		pressButton(Mouse_Right);
		releaseButton(Mouse_Right);
	}

	void Mouse::doubleRightClick() {
		rightClick();
		rightClick();
	}

	void Mouse::doubleLeftClick() {
		leftClick();
		leftClick();
	}

	void Mouse::leftDragClickTo(int toX, int toY) {
		pressButton(Mouse_Left);
		moveTo(toX, toY, true, false);
		releaseButton(Mouse_Left);
	}

	void Mouse::rightClickAt(int atX, int atY) {
		moveTo(atX, atY, false, false);
		rightClick();
	}

	void Mouse::rightDoubleClickAt(int atX, int atY) {
		moveTo(atX, atY, false, false);
		doubleRightClick();
	}
}