#include "HUD.h"
#include "UIManager.h"
#include "../Gameplay/Player.h"
#include "common/Items.h"
#include "raylib.h"
#include <string>

namespace TDS {

HUD::HUD(Game* game) : game(game) {}
HUD::~HUD() {}

void HUD::update(float dt) {}

void HUD::render() {
    auto theme = UIManager::getTheme();
    auto player = game->getPlayer();
    if (!player) return;

    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    // ========================================================================
    // Top-right: Player stats
    // ========================================================================
    const int statsX = screenWidth - 320;
    const int statsY = 20;
    const int statsWidth = 300;

    // Health bar
    Rectangle healthBounds = {(float)statsX, (float)statsY, (float)statsWidth, 30};
    UIManager::drawHealthBar(healthBounds, player->getHealth(), 100.0f);

    // Draw player level and money (if we had those stats)
    DrawText(TextFormat("Level: 1  Money: 0₽"), statsX, statsY + 40, 16, theme.text);

    // ========================================================================
    // Bottom-right: Weapon and ammo info
    // ========================================================================
    const int weaponX = screenWidth - 320;
    const int weaponY = screenHeight - 150;

    // Get current weapon
    uint16_t weaponId = player->getCurrentWeapon();
    const ItemDefinition* weaponDef = ItemDatabase::getItem(weaponId);

    if (weaponDef) {
        // Weapon panel
        Rectangle weaponPanel = {(float)weaponX, (float)weaponY, 300, 120};
        DrawRectangleRec(weaponPanel, Fade(theme.panelDark, 0.8f));
        DrawRectangleLinesEx(weaponPanel, 2, theme.accent);

        // Weapon name
        DrawText(weaponDef->name.c_str(), weaponX + 10, weaponY + 10, 20, theme.accent);

        // Ammo count (placeholder - we don't have ammo system yet)
        const char* ammoText = "30 / 120";
        DrawText(ammoText, weaponX + 10, weaponY + 40, 32, theme.text);
        DrawText("ROUNDS", weaponX + 10, weaponY + 75, 14, theme.textDark);

        // Fire mode indicator
        const char* fireMode = "AUTO";
        int fireModeWidth = MeasureText(fireMode, 16);
        DrawText(fireMode, weaponX + 290 - fireModeWidth, weaponY + 95, 16, theme.success);
    }

    // ========================================================================
    // Center: Crosshair
    // ========================================================================
    const int centerX = screenWidth / 2;
    const int centerY = screenHeight / 2;
    const int crosshairSize = player->getIsAiming() ? 8 : 12;
    const int crosshairGap = player->getIsAiming() ? 4 : 6;
    const int crosshairThickness = 2;

    Color crosshairColor = theme.text;

    // Horizontal line (left)
    DrawRectangle(centerX - crosshairSize - crosshairGap, centerY - crosshairThickness/2,
                  crosshairSize, crosshairThickness, crosshairColor);
    // Horizontal line (right)
    DrawRectangle(centerX + crosshairGap, centerY - crosshairThickness/2,
                  crosshairSize, crosshairThickness, crosshairColor);
    // Vertical line (top)
    DrawRectangle(centerX - crosshairThickness/2, centerY - crosshairSize - crosshairGap,
                  crosshairThickness, crosshairSize, crosshairColor);
    // Vertical line (bottom)
    DrawRectangle(centerX - crosshairThickness/2, centerY + crosshairGap,
                  crosshairThickness, crosshairSize, crosshairColor);

    // Center dot
    DrawCircle(centerX, centerY, 1, crosshairColor);

    // ========================================================================
    // Bottom-left: Status indicators
    // ========================================================================
    const int statusX = 20;
    const int statusY = screenHeight - 100;

    // Sprint indicator
    if (player->getIsSprinting()) {
        DrawText("SPRINTING", statusX, statusY, 16, theme.success);
    }

    // ADS indicator
    if (player->getIsAiming()) {
        DrawText("AIM DOWN SIGHTS", statusX, statusY + 25, 16, theme.accent);
    }

    // Reload indicator (if we had it)
    // DrawText("RELOADING...", statusX, statusY + 50, 16, theme.danger);

    // ========================================================================
    // Top-left: Mini notifications area
    // ========================================================================
    // DrawText("Kill: Enemy Scav", 20, 20, 16, theme.success);
    // DrawText("+ 150 XP", 20, 40, 14, theme.text);

    // ========================================================================
    // Debug info (FPS counter)
    // ========================================================================
    if (IsKeyDown(KEY_F3)) {
        DrawFPS(10, screenHeight - 30);
        DrawText(TextFormat("FOV: %.1f", player->getCurrentFOV()), 10, screenHeight - 50, 14, GREEN);
    }
}

void HUD::onStateChanged(GameState newState) {}

} // namespace TDS
