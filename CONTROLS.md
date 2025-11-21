# Game Controls - Extraction Shooter

Complete reference for all keyboard and mouse controls in the game.

---

## Main Menu Controls

### Login Screen
- **Mouse** - Click username/password fields to type
- **Tab** - Switch between fields
- **Enter** - Submit login/registration
- **Escape** - Exit game

### Main Menu
- **Mouse** - Click buttons to navigate
- **Escape** - Logout / Return to login

### Stash / Inventory Screen
- **Mouse** - Drag and drop items
- **Left Click** - Select item
- **Right Click** - Quick action (use/equip/drop)
- **Escape** - Close stash, return to main menu

---

## In-Game Controls

### Movement
| Key | Action |
|-----|--------|
| **W** | Move forward |
| **S** | Move backward |
| **A** | Strafe left |
| **D** | Strafe right |
| **Shift** | Sprint (hold) |
| **C** | Crouch/Stand (toggle) |
| **Space** | Jump |

**Movement Tips**:
- Sprint drains stamina faster
- Crouching reduces noise and makes you harder to spot
- Jumping makes noise - use sparingly near enemies

### Camera & Aiming
| Control | Action |
|---------|--------|
| **Mouse Move** | Look around / Aim |
| **Mouse Sensitivity** | Adjustable in settings (default: medium) |

**Camera Tips**:
- Third-person camera follows your character
- Camera automatically adjusts around obstacles
- Use mouse to scan for enemies and loot

### Combat
| Key/Button | Action |
|------------|--------|
| **Left Mouse Button** | Fire weapon (hold for automatic) |
| **Right Mouse Button** | Aim down sights (future feature) |
| **R** | Reload current weapon |
| **1-5** | Switch weapon (number keys) |
| **Tab** | Open/close inventory |
| **Middle Mouse** | Melee attack |

**Combat Tips**:
- Different weapons have different fire rates and damage
- Reload before entering combat when possible
- Listen for enemy gunfire to locate threats
- Headshots deal extra damage

### Interaction
| Key | Action |
|-----|--------|
| **F** | Loot nearby corpse/container |
| **E** | Extract at extraction zone |
| **G** | Drop item (in inventory) |

**Interaction Tips**:
- You must be within 3 meters to loot corpses
- Extraction zones glow with particles
- Looting takes time - watch your surroundings

### Inventory Management
| Key/Button | Action |
|------------|--------|
| **Tab** | Open/close inventory |
| **Mouse Drag** | Move items between slots |
| **Left Click** | Select/pick up item |
| **Right Click** | Quick equip/use item |
| **Delete/X** | Drop selected item |
| **Escape** | Close inventory |

**Inventory Tips**:
- Backpacks increase inventory space
- Some items stack, others don't
- Equipped items shown in loadout section
- Can't modify inventory while moving

### Communication (Future Feature)
| Key | Action |
|-----|--------|
| **V** | Push-to-talk (voice chat) |
| **Enter** | Text chat |
| **Y** | Team chat only |

---

## UI Navigation

### General UI
- **Left Click** - Select button/option
- **Escape** - Go back / Close window
- **Tab** - Switch between panels (context-dependent)

### Lobby System
| Action | How To |
|--------|--------|
| **Create Lobby** | Main menu → Enter Raid → Create |
| **Join Lobby** | Main menu → Enter Raid → Join → Select |
| **Ready Up** | Click "Ready" button in lobby |
| **Invite Friend** | Lobby → Friends → Click friend → Invite |
| **Start Queue** | Lobby owner → Click "Start Queue" |
| **Leave Lobby** | Click "Leave" button |

### Merchant System
| Action | How To |
|--------|--------|
| **Select Merchant** | Main menu → Merchants → Click merchant |
| **Buy Item** | Merchant screen → Select item → Buy |
| **Sell Item** | Merchant screen → Your stash → Select → Sell |
| **Compare Prices** | Switch between merchants to compare |

---

## HUD Elements

### In-Game HUD (displayed during raid)

```
┌─────────────────────────────────────────────┐
│ HP: ███████████░░  100/100                  │  ← Top-left: Health
│ Ammo: 30/120      AK-74                     │  ← Ammo count and weapon
│ ₽: 245,000                                  │  ← Money (Roubles)
│                                             │
│                                             │
│                   [CROSSHAIR]               │  ← Center: Aiming reticle
│                                             │
│                                             │
│                                             │
│                            [F] Loot Corpse  │  ← Bottom-right: Prompts
│ Time: 25:34               [E] Extract       │  ← Time remaining / Extract
└─────────────────────────────────────────────┘
```

### HUD Indicators

**Health Bar**:
- Green: 75-100 HP
- Yellow: 50-74 HP
- Orange: 25-49 HP
- Red: 1-24 HP

**Stamina** (future feature):
- Depletes when sprinting
- Regenerates when walking/standing

**Interaction Prompts**:
- Appear when near interactive objects
- Show required key to interact
- Disappear when out of range

---

## Advanced Controls

### Quick Actions
| Keys | Action |
|------|--------|
| **Alt + F4** | Exit game immediately |
| **F11** | Toggle fullscreen (future) |
| **F12** | Screenshot (future) |
| **~** | Console (debug mode only) |

### Emotes (Future Feature)
| Key | Action |
|-----|--------|
| **B** | Wave |
| **N** | Point |
| **M** | Thumbs up |

---

## Settings & Configuration

### Graphics Settings (in server_config.ini)
- Resolution: 1280x720 (default)
- FPS Cap: 60
- VSync: On

### Audio Settings
- Master Volume: 0-100%
- SFX Volume: 0-100%
- Music Volume: 0-100%
- 3D Audio: Enabled

### Mouse Settings
- Sensitivity: Low / Medium / High
- Invert Y-Axis: Off / On
- Raw Input: Enabled

---

## Control Customization

Currently, controls are **hardcoded** in the client. To customize:

1. Open `src/client/RaylibGameClient.h`
2. Find `handlePlayerMovement()` and `handleInput()` functions
3. Modify key bindings:

```cpp
// Example: Change sprint from Shift to Ctrl
if (IsKeyDown(KEY_LEFT_CONTROL)) {  // Was KEY_LEFT_SHIFT
    // Sprint logic
}
```

4. Rebuild the client

**Future Enhancement**: Config file for key bindings coming soon!

---

## Troubleshooting Controls

### "Keys not responding"
- Ensure game window has focus (click on it)
- Check if another application is capturing input
- Try restarting the game

### "Mouse too sensitive / not sensitive enough"
- Adjust DPI on your mouse
- Modify sensitivity in code (temporary)
- Wait for settings menu (future feature)

### "Can't loot corpses"
- Must be within 3 meters
- Press F (not E)
- Corpse may have already been looted

### "Can't extract"
- Must be in extraction zone (glowing area)
- Press E (not F)
- Check raid timer hasn't expired

---

## Tips & Tricks

### Movement Efficiency
- **Sprint in open areas** - Get to cover faster
- **Crouch near enemies** - Reduce detection
- **Jump over obstacles** - Navigate terrain faster

### Combat Tactics
- **Reload behind cover** - Never reload in the open
- **Burst fire** - More accurate than full auto
- **Strafe while shooting** - Harder to hit
- **Listen for footsteps** - Audio cues are critical

### Looting Strategy
- **Loot quickly** - Other players may be nearby
- **Check corpse gear first** - Best items usually equipped
- **Don't be greedy** - Sometimes extract early with decent loot
- **Use F key** - Faster than inventory dragging

### Extraction Tips
- **Know extraction locations** - Learn the map
- **Watch your time** - Don't get caught in raid
- **Clear area first** - Check for campers near extract
- **Sprint to extract** - Once clear, run straight there

---

## Default Control Summary Card

**Print this out for quick reference!**

```
╔════════════════════════════════════════════╗
║     EXTRACTION SHOOTER - CONTROLS          ║
╠════════════════════════════════════════════╣
║ MOVE      WASD     │ LOOT         F        ║
║ SPRINT    Shift    │ EXTRACT      E        ║
║ CROUCH    C        │ INVENTORY    Tab      ║
║ JUMP      Space    │ DROP         G        ║
║ SHOOT     LMB      │ RELOAD       R        ║
║ MELEE     MMB      │ WEAPON       1-5      ║
║ LOOK      Mouse    │ BACK/CLOSE   Escape   ║
╚════════════════════════════════════════════╝
```

---

## Platform-Specific Notes

### Windows
- All controls work as documented
- Use Windows key to minimize (not recommended during raid)

### Future: Linux/Mac
- Controls will remain the same
- Some keys may need remapping for non-QWERTY layouts

---

## Accessibility

### Future Accessibility Features (Planned)
- Colorblind modes
- Remappable keys
- One-handed control schemes
- Screen reader support
- Larger UI text options

---

**Need help?** See [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for gameplay issues or [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md) to get started.
