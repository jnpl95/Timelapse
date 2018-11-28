#pragma once
#include <Windows.h>

namespace MouseInput
{
	static long buttonTable[] = {
		MOUSEEVENTF_LEFTDOWN,
		MOUSEEVENTF_MIDDLEDOWN,
		MOUSEEVENTF_RIGHTDOWN
	};

	// Represents mouse device.
	class Mouse	{
	public:
		// Mouse button which can be pressed or released
		enum MouseButton { Mouse_Left, Mouse_Middle, Mouse_Right };

		// Moves mouse cursor in direction.
		static void move(int xDirection, int yDirection);

		// Gets current mouse position
		static void getMousePosition(int & xPos, int & yPos);

		// Moves mouse cursor to the specified position.
		static void moveTo(int x, int y);

		// Simulates mouse button press.
		static void pressButton(MouseButton button);

		// Simulates mouse button release.
		static void releaseButton(MouseButton button);

		// Simulates wheel up move 
		static void wheelUp();

		// Simulates wheel up down
		static void wheelDown();

		// Translates MouseButton to the platform's representation of the button
		static long translateMouseButton(MouseButton button);

		static void leftClick();
		static void middleClick();
		static void rightClick();
		static void doubleRightClick();
		static void doubleLeftClick();
		static void leftDragClickTo(int toX, int toY);
		static void rightClickAt(int atX, int atY);
		static void rightDoubleClickAt(int atX, int atY);
	};
}