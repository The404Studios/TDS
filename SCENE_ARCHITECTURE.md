# Scene-Based Architecture Documentation

## Overview

The UI system has been completely redesigned with a hierarchical, scene-based architecture that supports:
- **Scene Management** - Load/unload scenes independently
- **GameObject Hierarchy** - Parent/child relationships
- **Multi-threading** - ThreadPool for async operations
- **Task Scheduling** - Delayed and repeating tasks
- **Component-Based UI** - Reusable UI elements

## Architecture Diagram

```
SceneManager
├── ThreadPool (4 worker threads)
├── Scheduler (task management)
└── Scenes
    ├── LoginScene
    │   ├── Background Panel
    │   ├── Title Text
    │   └── Main Panel
    │       ├── Mode Title Text
    │       ├── Username Label/Field
    │       ├── Password Label/Field
    │       ├── Email Label/Field (register mode)
    │       ├── Submit Button
    │       │   └── Label Text (child)
    │       ├── Switch Mode Button
    │       │   └── Label Text (child)
    │       ├── Error Text
    │       └── Status Text
    │
    └── MainMenuScene
        ├── Background Panel
        ├── Title Text
        ├── Subtitle Text
        ├── Stats Panel
        │   ├── Stats Title
        │   └── Stat Labels/Values
        ├── Button Grid Panel
        │   ├── Lobby Button
        │   │   └── Label Text
        │   ├── Stash Button
        │   │   └── Label Text
        │   ├── Merchants Button
        │   │   └── Label Text
        │   └── Logout Button
        │       └── Label Text
        ├── Info Panel
        │   └── Info Text
        └── Version Text
```

## Core Systems

### 1. GameObject (GameObject.h/cpp)

Base class for all scene objects with hierarchy support.

**Features:**
- Parent/child relationships
- Transform (position, rotation, scale)
- Active/inactive states
- Tag system for searching
- Lifecycle methods (onLoad, onUnload, update, render)

**Example:**
```cpp
auto panel = std::make_shared<UIPanel>("MainPanel");
panel->setTag("mainPanel");

auto button = std::make_shared<UIButton>("SubmitButton", "Submit");
panel->addChild(button); // Button is now child of panel

// Find by name or tag
auto found = panel->findChild("SubmitButton");
auto tagged = panel->findChildWithTag("mainPanel");
```

### 2. Scene (Scene.h/cpp)

Container for root GameObjects with lifecycle management.

**Features:**
- Load/unload lifecycle
- Root object management
- Object finding by name/tag
- Custom load/unload callbacks

**Example:**
```cpp
auto scene = std::make_shared<Scene>("LoginScene");
scene->addRootObject(panel);
scene->setLoadCallback([]() {
    std::cout << "Scene loaded!" << std::endl;
});
```

### 3. SceneManager (SceneManager.h/cpp)

Manages all scenes with loading/unloading and transitions.

**Features:**
- Scene registration
- Synchronous and asynchronous loading
- Scene transitions
- Built-in ThreadPool and Scheduler
- Current scene tracking

**Example:**
```cpp
auto sceneManager = std::make_unique<SceneManager>();

// Register scenes
sceneManager->registerScene("Login", loginScene);
sceneManager->registerScene("MainMenu", mainMenuScene);

// Load scene
sceneManager->loadScene("Login");

// Async transition
sceneManager->transitionToAsync("MainMenu", true, []() {
    std::cout << "Transition complete!" << std::endl;
});
```

### 4. ThreadPool (ThreadPool.h/cpp)

Worker thread pool for parallel task execution.

**Features:**
- Configurable thread count (default: hardware_concurrency)
- Task queue with futures
- Wait for all tasks
- Pending task count

**Example:**
```cpp
auto& threadPool = sceneManager->getThreadPool();

// Submit async task
auto future = threadPool.enqueue([](int x) {
    return x * 2;
}, 42);

int result = future.get(); // result = 84
```

### 5. Scheduler (Scheduler.h/cpp)

Task scheduler for delayed and repeating tasks.

**Features:**
- Delayed execution
- Repeating tasks
- Next-frame scheduling
- Task cancellation

**Example:**
```cpp
auto& scheduler = sceneManager->getScheduler();

// Run after 2 seconds
scheduler.scheduleTask([]() {
    std::cout << "Delayed task!" << std::endl;
}, 2.0f);

// Repeat every 0.5 seconds
auto task = scheduler.scheduleRepeating([]() {
    std::cout << "Repeating!" << std::endl;
}, 0.5f);

// Cancel later
scheduler.cancelTask(task);
```

## UI Component System

### UIElement (UIElement.h/cpp)

Base class for all UI components.

**Features:**
- Size (width, height)
- Color with alpha
- Visibility
- Mouse interaction (containsPoint, onClick, onMouseEnter/Exit)
- Hover state

### UIPanel (UIPanel.h/cpp)

Rectangular container for other UI elements.

**Features:**
- Border (color, width, draw/hide)
- Shadow (offset, alpha)
- Rounded corners (optional)
- Filled background

**Example:**
```cpp
auto panel = std::make_shared<UIPanel>("Panel");
panel->setSize(0.5f, 0.3f);
panel->setColor(Color(0.12f, 0.12f, 0.16f, 0.95f));
panel->setShadow(true, 0.02f, 0.02f, 0.5f);
panel->setDrawBorder(true);
panel->setBorderColor(Color(0.4f, 0.35f, 0.25f, 1.0f));
```

### UIText (UIText.h/cpp)

Text rendering component.

**Features:**
- Font size
- Alignment (LEFT, CENTER, RIGHT)
- Shadow support
- Static font initialization

**Example:**
```cpp
auto text = std::make_shared<UIText>("Title", "MAIN MENU");
text->setFontSize(2.0f);
text->setAlignment(TextAlign::CENTER);
text->setColor(Color(1.0f, 0.85f, 0.5f, 1.0f));
text->setShadow(true, 0.015f, 0.015f, 0.5f);
```

### UIButton (UIButton.h/cpp)

Interactive button with text label.

**Features:**
- Auto-created text label (as child)
- Click callback
- Hover effects (colors, animations)
- Normal/hover color states
- Animated pulse on hover

**Example:**
```cpp
auto button = std::make_shared<UIButton>("LoginButton", "LOGIN");
button->setSize(0.5f, 0.1f);
button->setOnClick([]() {
    std::cout << "Button clicked!" << std::endl;
});
button->setNormalColor(Color(0.15f, 0.15f, 0.2f, 0.95f));
button->setHoverColor(Color(0.25f, 0.22f, 0.18f, 0.95f));
```

## Scene Examples

### LoginScene

**Hierarchy:**
- Background (UIPanel)
- Title (UIText)
- Main Panel (UIPanel)
  - Field labels and inputs (UIText)
  - Submit button (UIButton with child label)
  - Switch mode button (UIButton with child label)
  - Error/status text (UIText)

**Features:**
- Login/Register mode switching
- Email validation
- Network packet processing
- Mouse interaction
- Animated title and status

### MainMenuScene

**Hierarchy:**
- Background (UIPanel)
- Title/Subtitle (UIText)
- Stats Panel (UIPanel with stat displays)
- Button Grid Panel (UIPanel)
  - 4 buttons in 2x2 grid
  - Each button has label as child
- Info Panel (UIPanel with dynamic text)
- Version text (UIText)

**Features:**
- 2x2 button grid layout
- Player stats display
- Dynamic info panel
- Navigation callbacks
- Hover-based descriptions

## Benefits of New Architecture

### 1. Memory Management
- Scenes can be loaded/unloaded independently
- No memory leaks from proper hierarchy cleanup
- Smart pointers for automatic memory management

### 2. Organization
- Clear parent/child relationships
- Tag-based object lookup
- Separated concerns (scene, UI, logic)

### 3. Performance
- ThreadPool for async operations
- Scheduler for efficient task management
- Render only active objects

### 4. Flexibility
- Easy to add new scenes
- Reusable UI components
- Modular design

### 5. Maintainability
- Clean separation of systems
- Each class has single responsibility
- Easy to debug with hierarchy

## Migration from Old System

**Old Code:**
```cpp
LoginUI* loginUI = new LoginUI(networkClient);
uiManager->setState(UIState::LOGIN, loginUI);
```

**New Code:**
```cpp
auto loginScene = std::make_shared<LoginScene>(networkClient);
sceneManager->registerScene("Login", loginScene);
sceneManager->loadScene("Login");
```

## Future Enhancements

### Potential Additions:
1. **Animation System** - Tween animations for transforms
2. **Layout System** - Automatic layout (grid, flex, etc.)
3. **Input System** - Dedicated input manager
4. **Resource Manager** - Texture/font loading
5. **Scene Transitions** - Fade in/out effects
6. **UI Events** - Event bus for communication
7. **Serialization** - Save/load scenes from JSON
8. **Prefabs** - Reusable scene templates

## Performance Metrics

- **ThreadPool**: 4 worker threads (configurable)
- **Scene Loading**: Async with callbacks
- **Update Loop**: ~60 FPS target
- **Memory**: Scenes unload when not needed

## File Structure

```
src/client/ui/
├── GameObject.h/cpp          # Base hierarchical object
├── Scene.h/cpp               # Scene container
├── SceneManager.h/cpp        # Scene management
├── ThreadPool.h/cpp          # Multi-threading
├── Scheduler.h/cpp           # Task scheduling
├── UIElement.h/cpp           # Base UI component
├── UIPanel.h/cpp             # Panel container
├── UIText.h/cpp              # Text rendering
├── UIButton.h/cpp            # Interactive button
├── LoginScene.h/cpp          # Login scene
└── MainMenuScene.h/cpp       # Main menu scene

src/client/
└── main.cpp                  # Entry point with SceneManager
```

## Summary

The new architecture provides:
✅ **Scene-based structure** - Load/unload independently
✅ **Hierarchy system** - Parent/child relationships
✅ **Multi-threading** - Async task execution
✅ **Scheduling** - Delayed/repeating tasks
✅ **Component-based UI** - Reusable elements
✅ **Clean separation** - Each system has clear responsibility
✅ **Memory efficient** - Smart pointers and proper cleanup
✅ **Extensible** - Easy to add new scenes/components

This is a production-ready architecture that scales well for complex UI systems!
