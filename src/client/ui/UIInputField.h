#pragma once

#include "UIPanel.h"
#include "UIText.h"
#include <string>
#include <functional>

// Input field type
enum class InputFieldType {
    TEXT,           // Normal text
    PASSWORD,       // Masked input
    NUMBER,         // Only numbers
    EMAIL           // Email validation
};

// UI Input Field - Editable text input
class UIInputField : public UIPanel {
public:
    UIInputField(const std::string& name = "InputField", const std::string& placeholder = "");

    void render() override;
    void update(float deltaTime) override;

    // Input handling
    void handleChar(char c);
    void handleBackspace();
    void handleEnter();

    // Text management
    void setText(const std::string& txt);
    std::string getText() const { return text; }
    void clear() { text.clear(); updateDisplayText(); }

    // Placeholder
    void setPlaceholder(const std::string& ph) { placeholder = ph; updateDisplayText(); }
    const std::string& getPlaceholder() const { return placeholder; }

    // Input type
    void setInputType(InputFieldType type) { inputType = type; updateDisplayText(); }
    InputFieldType getInputType() const { return inputType; }

    // Focus
    void setFocused(bool focus);
    bool isFocused() const { return focused; }

    // Callbacks
    void setOnTextChanged(std::function<void(const std::string&)> callback) { onTextChanged = callback; }
    void setOnEnter(std::function<void(const std::string&)> callback) { onEnterCallback = callback; }

    // Validation
    void setMaxLength(size_t max) { maxLength = max; }
    size_t getMaxLength() const { return maxLength; }
    void setAllowedChars(const std::string& chars) { allowedChars = chars; }
    bool isValid() const;

    // Cursor
    void setCursorPosition(size_t pos);
    size_t getCursorPosition() const { return cursorPosition; }

    // Selection
    void selectAll();
    void clearSelection();
    void deleteSelection();
    bool hasSelection() const { return selectionStart != selectionEnd; }

    // Colors
    void setTextColor(const Color& col) { textColor = col; }
    void setPlaceholderColor(const Color& col) { placeholderColor = col; }
    void setCursorColor(const Color& col) { cursorColor = col; }
    void setSelectionColor(const Color& col) { selectionColor = col; }

private:
    void updateDisplayText();
    void renderCursor();
    void renderSelection();
    std::string getMaskedText() const;
    bool isCharAllowed(char c) const;

    std::string text;
    std::string placeholder;
    InputFieldType inputType;
    bool focused;
    size_t maxLength;
    std::string allowedChars;

    // Display
    std::shared_ptr<UIText> displayText;
    Color textColor;
    Color placeholderColor;
    Color cursorColor;
    Color selectionColor;

    // Cursor
    size_t cursorPosition;
    float cursorBlinkTime;
    bool cursorVisible;

    // Selection
    size_t selectionStart;
    size_t selectionEnd;

    // Callbacks
    std::function<void(const std::string&)> onTextChanged;
    std::function<void(const std::string&)> onEnterCallback;
};
