#include "UIInputField.h"
#include <cmath>
#include <algorithm>

UIInputField::UIInputField(const std::string& name, const std::string& placeholder)
    : UIPanel(name), text(""), placeholder(placeholder), inputType(InputFieldType::TEXT),
      focused(false), maxLength(256), allowedChars(""),
      cursorPosition(0), cursorBlinkTime(0.0f), cursorVisible(true),
      selectionStart(0), selectionEnd(0),
      onTextChanged(nullptr), onEnterCallback(nullptr) {

    // Set default colors
    textColor = Color(1.0f, 1.0f, 1.0f, 1.0f);
    placeholderColor = Color(0.5f, 0.5f, 0.5f, 0.7f);
    cursorColor = Color(1.0f, 1.0f, 1.0f, 0.9f);
    selectionColor = Color(0.3f, 0.5f, 0.8f, 0.5f);

    // Set default appearance
    setColor(Color(0.1f, 0.1f, 0.15f, 0.95f));
    setDrawBorder(true);
    setBorderColor(Color(0.3f, 0.3f, 0.4f, 1.0f));
    setBorderWidth(2.0f);

    // Create display text
    displayText = std::make_shared<UIText>("DisplayText", placeholder);
    displayText->setFontSize(1.0f);
    displayText->setColor(placeholderColor);
    displayText->getTransform().x = 0.02f;
    displayText->setSize(width, height);
    addChild(displayText);

    updateDisplayText();
}

void UIInputField::setFocused(bool focus) {
    if (focused == focus) return;

    focused = focus;

    if (focused) {
        // Focus gained - highlight border
        setBorderColor(Color(0.5f, 0.6f, 0.8f, 1.0f));
        setBorderWidth(3.0f);
        cursorVisible = true;
        cursorBlinkTime = 0.0f;
    } else {
        // Focus lost - normal border
        setBorderColor(Color(0.3f, 0.3f, 0.4f, 1.0f));
        setBorderWidth(2.0f);
        clearSelection();
    }
}

void UIInputField::setText(const std::string& txt) {
    text = txt;
    if (text.length() > maxLength) {
        text = text.substr(0, maxLength);
    }
    cursorPosition = std::min(cursorPosition, text.length());
    clearSelection();
    updateDisplayText();

    if (onTextChanged) {
        onTextChanged(text);
    }
}

void UIInputField::handleChar(char c) {
    if (!focused) return;
    if (text.length() >= maxLength) return;
    if (!isCharAllowed(c)) return;

    // Delete selection if exists
    if (hasSelection()) {
        deleteSelection();
    }

    // Insert character at cursor
    text.insert(cursorPosition, 1, c);
    cursorPosition++;
    updateDisplayText();

    if (onTextChanged) {
        onTextChanged(text);
    }
}

void UIInputField::handleBackspace() {
    if (!focused) return;

    if (hasSelection()) {
        deleteSelection();
    } else if (cursorPosition > 0) {
        text.erase(cursorPosition - 1, 1);
        cursorPosition--;
        updateDisplayText();

        if (onTextChanged) {
            onTextChanged(text);
        }
    }
}

void UIInputField::handleEnter() {
    if (!focused) return;

    if (onEnterCallback) {
        onEnterCallback(text);
    }
}

void UIInputField::updateDisplayText() {
    if (!displayText) return;

    std::string displayString;

    if (text.empty()) {
        // Show placeholder
        displayString = placeholder;
        displayText->setColor(placeholderColor);
    } else {
        // Show text (masked if password)
        displayString = getMaskedText();
        displayText->setColor(textColor);
    }

    displayText->setText(displayString);
    displayText->setSize(width - 0.04f, height);
    displayText->getTransform().y = height / 2.0f - 0.02f;
}

std::string UIInputField::getMaskedText() const {
    if (inputType == InputFieldType::PASSWORD) {
        return std::string(text.length(), '*');
    }
    return text;
}

bool UIInputField::isCharAllowed(char c) const {
    // Control characters not allowed (except handled separately)
    if (c < 32 || c > 126) return false;

    // Check input type restrictions
    if (inputType == InputFieldType::NUMBER) {
        if (c >= '0' && c <= '9') return true;
        if (c == '.' || c == '-') return true;  // Allow decimal and negative
        return false;
    }

    // Check allowed chars filter
    if (!allowedChars.empty()) {
        return allowedChars.find(c) != std::string::npos;
    }

    return true;
}

bool UIInputField::isValid() const {
    if (text.empty()) return false;

    if (inputType == InputFieldType::EMAIL) {
        // Simple email validation
        size_t atPos = text.find('@');
        if (atPos == std::string::npos || atPos == 0 || atPos == text.length() - 1) {
            return false;
        }
        size_t dotPos = text.find('.', atPos);
        if (dotPos == std::string::npos || dotPos == atPos + 1 || dotPos == text.length() - 1) {
            return false;
        }
        return true;
    }

    if (inputType == InputFieldType::NUMBER) {
        // Check if valid number
        try {
            std::stod(text);
            return true;
        } catch (...) {
            return false;
        }
    }

    return true;
}

void UIInputField::setCursorPosition(size_t pos) {
    cursorPosition = std::min(pos, text.length());
    cursorBlinkTime = 0.0f;
    cursorVisible = true;
}

void UIInputField::selectAll() {
    selectionStart = 0;
    selectionEnd = text.length();
}

void UIInputField::clearSelection() {
    selectionStart = 0;
    selectionEnd = 0;
}

void UIInputField::deleteSelection() {
    if (!hasSelection()) return;

    size_t start = std::min(selectionStart, selectionEnd);
    size_t end = std::max(selectionStart, selectionEnd);

    text.erase(start, end - start);
    cursorPosition = start;
    clearSelection();
    updateDisplayText();

    if (onTextChanged) {
        onTextChanged(text);
    }
}

void UIInputField::update(float deltaTime) {
    // Update cursor blink
    if (focused) {
        cursorBlinkTime += deltaTime;
        if (cursorBlinkTime >= 1.0f) {
            cursorBlinkTime = 0.0f;
            cursorVisible = !cursorVisible;
        }
    }

    UIPanel::update(deltaTime);
}

void UIInputField::render() {
    UIPanel::render();

    if (focused && !text.empty()) {
        // Render selection
        if (hasSelection()) {
            renderSelection();
        }

        // Render cursor
        if (cursorVisible) {
            renderCursor();
        }
    }
}

void UIInputField::renderCursor() {
    UITransform worldTrans = getWorldTransform();
    float x = worldTrans.x;
    float y = worldTrans.y;

    // Calculate cursor position based on text
    float charWidth = 0.0075f * displayText->getFontSize();
    float cursorX = x + 0.02f + (cursorPosition * charWidth);
    float cursorY = y + 0.02f;
    float cursorH = height - 0.04f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(cursorColor.r, cursorColor.g, cursorColor.b, cursorColor.a);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(cursorX, cursorY);
    glVertex2f(cursorX, cursorY + cursorH);
    glEnd();
    glLineWidth(1.0f);
}

void UIInputField::renderSelection() {
    if (!hasSelection()) return;

    UITransform worldTrans = getWorldTransform();
    float x = worldTrans.x;
    float y = worldTrans.y;

    size_t start = std::min(selectionStart, selectionEnd);
    size_t end = std::max(selectionStart, selectionEnd);

    float charWidth = 0.0075f * displayText->getFontSize();
    float selX = x + 0.02f + (start * charWidth);
    float selW = (end - start) * charWidth;
    float selY = y + 0.02f;
    float selH = height - 0.04f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(selectionColor.r, selectionColor.g, selectionColor.b, selectionColor.a);
    glBegin(GL_QUADS);
    glVertex2f(selX, selY);
    glVertex2f(selX + selW, selY);
    glVertex2f(selX + selW, selY + selH);
    glVertex2f(selX, selY + selH);
    glEnd();
}
