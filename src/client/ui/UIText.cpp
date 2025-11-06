#include "UIText.h"

// Static members
GLuint UIText::g_fontBase = 0;
HDC UIText::g_hDC = nullptr;
bool UIText::g_fontInitialized = false;

UIText::UIText(const std::string& name, const std::string& text)
    : UIElement(name), text(text), fontSize(1.0f), alignment(TextAlign::LEFT),
      hasShadow(false), shadowOffsetX(0.01f), shadowOffsetY(0.01f), shadowAlpha(0.4f) {
}

void UIText::initFont(HDC hDC) {
    g_hDC = hDC;

    HFONT hFont = CreateFontA(
        -24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_MODERN | FIXED_PITCH,
        "Courier New"
    );

    HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
    g_fontBase = glGenLists(256);
    wglUseFontBitmapsA(hDC, 0, 256, g_fontBase);
    SelectObject(hDC, hOldFont);
    DeleteObject(hFont);
    g_fontInitialized = true;
}

void UIText::renderText(const std::string& txt, float x, float y, float size) {
    if (!g_fontInitialized) return;

    glPushMatrix();
    glScalef(size, size, 1.0f);
    glPushAttrib(GL_LIST_BIT);
    glListBase(g_fontBase);
    glRasterPos2f(x / size, y / size);
    glCallLists((GLsizei)txt.length(), GL_UNSIGNED_BYTE, txt.c_str());
    glPopAttrib();
    glPopMatrix();
}

void UIText::render() {
    if (!visible || !active || text.empty()) return;

    Transform worldTrans = getWorldTransform();
    float x = worldTrans.x;
    float y = worldTrans.y;

    // Calculate alignment offset
    float charWidth = 0.0075f * fontSize;
    float textWidth = text.length() * charWidth;

    if (alignment == TextAlign::CENTER) {
        x += (width - textWidth) / 2.0f;
    } else if (alignment == TextAlign::RIGHT) {
        x += width - textWidth;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw shadow
    if (hasShadow) {
        glColor4f(0.0f, 0.0f, 0.0f, shadowAlpha);
        renderText(text, x + shadowOffsetX, y - shadowOffsetY, fontSize);
    }

    // Draw text
    setGLColor(color);
    renderText(text, x, y, fontSize);

    // Render children
    UIElement::render();
}
