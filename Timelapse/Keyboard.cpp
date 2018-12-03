#include <stdexcept>
#include <iostream>
#include "Keyboard.h"

namespace KeyboardInput {
	static long keyTable[] = {
	0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, // A - I
	0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x52, // J - R
	0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, // S - Z
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // 0 - 9

	VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
	VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20, VK_F21, VK_F22, VK_F23, VK_F24,
	VK_ESCAPE, VK_SPACE, VK_RETURN, VK_BACK, VK_TAB,
	VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL, VK_LMENU, VK_RMENU, VK_LWIN, VK_RWIN, VK_APPS,
	VK_CAPITAL, VK_NUMLOCK, VK_SCROLL,
	VK_SNAPSHOT, VK_PAUSE,
	VK_INSERT, VK_DELETE, VK_PRIOR, VK_NEXT, VK_HOME, VK_END,
	VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN,
	VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5, VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9,
	VK_ADD, VK_SUBTRACT, VK_MULTIPLY, VK_DIVIDE, VK_DECIMAL, VK_RETURN
	};

	Key_base::Key_base() {
		code_ = 0;
		name_ = "<no key>";
	}

	unsigned long Key_base::translateKey(KeyType key) {
		return keyTable[key];
	}

	unsigned int Key_base::code() const {
		return code_;
	}

	const std::string& Key_base::name() const {
		return name_;
	}

	Key_win::Key_win() {
		virtualKey_ = 0;
	}

	Key_win::Key_win(KeyType type) {
		if (type == Key_NoKey) {
			virtualKey_ = 0;
			code_ = 0;
			name_ = "<no key>";
		}
		else {
			virtualKey_ = (WORD)translateKey(type);

			Key_win tmpKey(virtualKey_);
			code_ = tmpKey.code_;
			name_ = tmpKey.name_;
		}
	}

	Key_win::Key_win(MSG* message) {
		switch (message->message) {
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			virtualKey_ = message->wParam;
			break;
		default:
			std::logic_error("Cannot get key from non-key message");
			break;
		}
		Key_win tmpKey(virtualKey_);
		code_ = tmpKey.code_;
		name_ = tmpKey.name_;
	}

	Key_win::Key_win(WORD virtualKey) :
		virtualKey_(virtualKey) {
		code_ = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
		LONG lParam = code_;
		switch (virtualKey) {
		case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN: // arrow keys
		case VK_PRIOR: case VK_NEXT: // page up and page down
		case VK_END: case VK_HOME:
		case VK_INSERT: case VK_DELETE:
		case VK_DIVIDE: // numpad slash
		case VK_NUMLOCK: {
			lParam |= 0x100; // set extended bit
			break;
			}
		}
		char name[128];
		if (GetKeyNameText(lParam << 16, (LPWSTR)name, 128)) {
			name_ = std::string(name);
		}
		else {
			std::cerr << "Cannot get key name";
			name_ = "<unknown key>";
		}
	}

	WORD Key_win::virtualKey() const {
		return virtualKey_;
	}

	void Keyboard::sendKeyEvent_(Key key, bool isPress) {
		if (key.code() == 0) {
			std::cerr << "Cannot send <no key> event" << std::endl;
		}
		else {
			INPUT input;
			ZeroMemory(&input, sizeof(INPUT));
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = key.virtualKey();
			input.ki.dwFlags = (isPress) ? 0 : KEYEVENTF_KEYUP;
			SendInput(1, &input, sizeof(INPUT));
		}
	}

	void Keyboard::pressKey(Key key) {
		sendKeyEvent_(key, true);
	}

	void Keyboard::releaseKey(Key key) {
		sendKeyEvent_(key, false);
	}

	 void Keyboard::spamKey(Key key, int times) {
		if (times > 0) {
			int i;
			for (i = 0; i < times; i++) {
				pressKey(key);
				releaseKey(key);
			}
		}
		pressKey(key);
		releaseKey(key);
	}

	void Keyboard::holdKeyDown(Key key, int time) {
		pressKey(key);
		Sleep(time);
		releaseKey(key);
	}
}