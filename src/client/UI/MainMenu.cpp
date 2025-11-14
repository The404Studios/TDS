#include "MainMenu.h"
#include "UIManager.h"
#include "raygui.h"

namespace TDS {

MainMenu::MainMenu(Game* game)
    : game(game)
    , selectedOption(0)
    , animProgress(0.0f)
{
}

MainMenu::~MainMenu() {}

void MainMenu::update(float dt) {
    animProgress += dt;
    
    // Keyboard navigation
    if (IsKeyPressed(KEY_UP)) {
        selectedOption = (selectedOption - 1 + 4) % 4;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        selectedOption = (selectedOption + 1) % 4;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        handleSelection();
    }
}

void MainMenu::render() {
    int screenWidth = game->getScreenWidth();
    int screenHeight = game->getScreenHeight();
    
    // Background
    DrawRectangleGradientV(0, 0, screenWidth, screenHeight,
                          UIManager::getTheme().background,
                          ColorBrightness(UIManager::getTheme().background, -0.3f));
    
    // Title
    const char* title = "MAIN MENU";
    int titleSize = 50;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (screenWidth - titleWidth) / 2, 100, titleSize, UIManager::getTheme().accent);
    
    // Menu panel
    Rectangle menuPanel = {
        (float)(screenWidth / 2 - 300),
        (float)(screenHeight / 2 - 250),
        600,
        500
    };
    
    UIManager::drawPanel(menuPanel, nullptr);
    
    float btnY = menuPanel.y + 50;
    float btnWidth = 500;
    float btnHeight = 70;
    float spacing = 20;
    
    const char* options[] = {
        "ENTER RAID",
        "STASH",
        "MERCHANTS",
        "LOGOUT"
    };
    
    for (int i = 0; i < 4; i++) {
        Rectangle btnRect = {
            menuPanel.x + (menuPanel.width - btnWidth) / 2,
            btnY,
            btnWidth,
            btnHeight
        };
        
        bool isSelected = (i == selectedOption);
        Color btnColor = isSelected ? UIManager::getTheme().accent : UIManager::getTheme().panelLight;
        Color hoverColor = UIManager::getTheme().accentHover;
        
        if (UIManager::drawButtonEx(btnRect, options[i], btnColor, hoverColor)) {
            selectedOption = i;
            handleSelection();
        }
        
        btnY += btnHeight + spacing;
    }
    
    // Player stats
    DrawText("LEVEL: 15  |  RAIDS: 42  |  SURVIVAL RATE: 48%",
             menuPanel.x + 50, menuPanel.y + menuPanel.height - 40,
             18, UIManager::getTheme().textDark);
}

void MainMenu::onStateChanged(GameState newState) {
    selectedOption = 0;
    animProgress = 0.0f;
}

void MainMenu::handleSelection() {
    switch (selectedOption) {
        case 0: // Enter Raid
            TraceLog(LOG_INFO, "Entering raid...");
            game->setState(GameState::IN_GAME);
            break;
        case 1: // Stash
            TraceLog(LOG_INFO, "Opening stash...");
            // TODO: Open stash UI
            break;
        case 2: // Merchants
            TraceLog(LOG_INFO, "Opening merchants...");
            // TODO: Open merchant UI
            break;
        case 3: // Logout
            TraceLog(LOG_INFO, "Logging out...");
            game->setState(GameState::LOGIN);
            break;
    }
}

} // namespace TDS
