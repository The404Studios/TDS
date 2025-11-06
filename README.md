# Neural Civilization - Multiplayer Strategy Simulation v2.0

A sophisticated real-time strategy simulation powered by neural networks with full client-server multiplayer architecture, advanced gameplay features, and emergent AI behavior.

## 🌟 Major Features

### Client-Server Architecture
- **Dedicated Server**: Authoritative game state management
- **Client Application**: OpenGL-based 3D visualization with intuitive controls
- **Network Synchronization**: Real-time state updates via TCP/IP
- **Low Latency**: Optimized packet serialization and delta compression

### Advanced Gameplay Systems

#### 🤝 Diplomacy System
- **Relations**: War, Peace, Trade Agreements, Non-Aggression Pacts, Alliances
- **Trust Levels**: Dynamic trust system (0-100) affecting diplomatic actions
- **Actions**: Declare War, Offer Peace, Propose Trade/Alliance, Demand Tribute, Offer Gifts
- **Consequences**: Diplomatic actions affect faction relationships and AI behavior

#### 🔬 Technology Research
- **Military Technologies**: Bronze/Iron/Steel Weapons, Archery, Cavalry, Siege Weapons
- **Economic Technologies**: Agriculture, Irrigation, Mining, Metallurgy, Currency, Banking
- **Civic Technologies**: Writing, Philosophy, Mathematics, Engineering, Medicine, Education
- **Defensive Technologies**: Masonry, Construction, Fortifications, Walls
- **Tech Tree**: Prerequisites system with meaningful gameplay bonuses

#### 💰 Trading & Market System
- **Dynamic Pricing**: Supply and demand affect resource prices
- **Trade Offers**: Complex multi-resource trading between factions
- **Global Market**: Real-time price updates based on production/consumption
- **Resource Types**: Food, Wood, Stone, Gold, Weapons, Luxury Goods

#### ⛈️ Weather & Seasonal Systems
- **Weather Types**: Clear, Rain, Storm, Snow, Fog, Drought
- **Weather Effects**: Impact movement speed, visibility, farming, and combat
- **Seasonal Cycles**: Spring, Summer, Fall, Winter (3-minute cycles)
- **Seasonal Modifiers**: Farming productivity, movement penalties, visual effects

#### ⚔️ Advanced Combat
- **Unit Formations**: Scattered, Line, Column, Circle, Wedge
- **Morale System**: Affects combat effectiveness
- **Experience**: Units gain experience from battles
- **Combat Units**: Group agents into coordinated military units
- **Visual Effects**: Combat animations and particle systems

#### 📜 Mission/Quest System
- **Mission Types**: Destroy Building, Kill Agents, Collect Resources, Defend Location, Explore Area, Trade Routes
- **Rewards**: Resources and experience bonuses
- **Time Limits**: Optional timed missions
- **Progress Tracking**: Real-time mission status updates

#### 👥 Population Management
- **Happiness System**: Influenced by health, literacy, resources, and war status
- **Growth Mechanics**: Birth/death rates, immigration/emigration
- **Health & Literacy**: Improved through funding and technologies
- **Policy Management**: Education, Healthcare, Infrastructure, Science funding

### Neural Network AI
- **Evolutionary Learning**: Genetic algorithms optimize agent behavior across generations
- **Role-Based Decisions**: Citizens, Workers, Soldiers, Government officials
- **Adaptive Strategies**: AI learns from successful behaviors
- **Multi-Layer Networks**: Complex decision-making with memory systems

### Enhanced Visualization
- **3D OpenGL Graphics**: Smooth rendering of world, agents, and buildings
- **Particle Effects**: Combat, resource gathering, construction visual feedback
- **Text UI**: Comprehensive information display with readable fonts
- **Camera Controls**: Full 3D camera with rotation, pan, and zoom
- **Weather Visualization**: Visual effects for different weather conditions

## 🏗️ Project Structure

```
TDS/
├── Client (NeuralCivilization.vcxproj)
│   ├── main.cpp                    - Client application entry point
│   ├── CivilizationAI.cpp/.h      - Core AI and simulation logic
│   ├── NetworkManager.cpp/.h      - Client networking
│   ├── GameplayFeatures.cpp/.h    - Advanced gameplay systems
│   └── GL/                        - OpenGL headers
│
├── Server/ (NeuralCivilizationServer.vcxproj)
│   └── ServerMain.cpp             - Dedicated server entry point
│
├── NeuralCivilization.sln         - Visual Studio solution
└── README.md                      - This file
```

## 🚀 Building the Project

### Prerequisites
- Windows 10/11
- Visual Studio 2022 (or 2019)
- Windows SDK 10.0
- C++17 support

### Build Steps

1. **Open Solution**
   ```
   Open NeuralCivilization.sln in Visual Studio
   ```

2. **Build Configuration**
   - Select `Debug x64` or `Release x64`
   - Both Client and Server projects are configured

3. **Build Both Projects**
   - Build → Build Solution (Ctrl+Shift+B)
   - Or right-click on each project and select "Build"

4. **Output Locations**
   - Client: `x64/Debug/NeuralCivilization.exe` or `x64/Release/NeuralCivilization.exe`
   - Server: `Server/x64/Debug/NeuralCivilizationServer.exe` or `Server/x64/Release/NeuralCivilizationServer.exe`

## 🎮 Running the Game

### Singleplayer Mode (Standalone Client)

1. Run `NeuralCivilization.exe`
2. The simulation starts immediately in singleplayer mode
3. All features work locally without networking

### Multiplayer Mode

#### 1. Start the Server
```bash
cd Server/x64/Debug
NeuralCivilizationServer.exe [port]
```
Default port: 27015

The server will:
- Initialize the game world
- Start listening for client connections
- Run authoritative simulation
- Display statistics every 10 seconds

#### 2. Start the Client(s)
```bash
cd x64/Debug
NeuralCivilization.exe
```

In the client:
- Press `N` to toggle Multiplayer Mode
- Client will automatically attempt to connect to `127.0.0.1:27015`
- Once connected, you'll see "ONLINE (Ping: XXms)" in the top bar

## 🎮 Controls

### Keyboard Controls
- **N** - Toggle Multiplayer Mode (connect/disconnect from server)
- **TAB** - Open/Close detailed statistics menu
- **SPACE** - Pause/Resume simulation
- **1-5** - Change simulation speed (1x, 2x, 3x, 5x, 10x)
- **C** - Change selected agent's role
- **R** - Reset world (generate new simulation)
- **ESC** - Exit application

### Mouse Controls
- **Left Click** - Select agent or building
- **Ctrl + Left Drag** - Rotate camera
- **Right Drag** - Pan camera
- **Mouse Wheel** - Zoom in/out

## 📊 UI Information

### Top Bar
- Generation number
- Simulation speed
- Pause/Run status
- Multiplayer connection status & ping
- Weather and season information

### Detailed Menu (TAB)
- **Per-Faction Statistics**:
  - Population and building counts
  - Role distribution (Citizens, Workers, Soldiers, Governors)
  - Resource stockpiles (Food, Wood, Stone, Gold)
  - Expansion readiness indicators

- **Selected Entity Info**:
  - Agent: Health, Energy, Age, Kills, Carried Resources
  - Building: Type, Health, Occupancy, Position

### Bottom Right
- Control hints and keyboard shortcuts

## 🌐 Network Architecture

### Client-Server Model
- **Server Authority**: All game logic runs on server
- **Client Prediction**: Smooth local visualization
- **State Synchronization**: Delta updates every 100ms
- **Packet Types**:
  - Connection management (Connect, Disconnect, Ping/Pong)
  - Game state (World State, Agent/Building Updates)
  - Player commands (Input, Role Changes, Build Orders)
  - Events (Combat, Spawns, Deaths, Diplomacy, Trade)

### Packet Structure
```cpp
PacketType (1 byte) | Size (4 bytes) | Data (variable)
```

### Network Statistics
Server displays:
- Connected clients
- Packets sent/received
- Bytes transferred
- Current world state

## 🎯 Gameplay Tips

1. **Early Game**
   - Focus on building houses for population growth
   - Balance worker-to-citizen ratio for resource gathering
   - Research Agriculture for better food production

2. **Mid Game**
   - Establish trade agreements with other factions
   - Research military technologies before conflicts
   - Form alliances for mutual defense

3. **Late Game**
   - Maintain happiness through education and healthcare funding
   - Coordinate combat units with proper formations
   - Use diplomacy to isolate enemies

4. **Weather Strategy**
   - Plan attacks during clear weather
   - Increase farming during rainy seasons
   - Build defenses before winter

## 🔧 Configuration

### Server Configuration (ServerMain.cpp)
```cpp
int port = 27015;              // Default port
tickRate = 1.0f / 60.0f;       // 60 Hz simulation
WORLD_SIZE = 500;              // World dimensions
MAX_AGENTS_PER_FACTION = 100;  // Population cap
```

### Client Configuration (main.cpp)
```cpp
g_serverAddress = "127.0.0.1"; // Server IP
g_serverPort = 27015;          // Server port
g_windowWidth = 1600;          // Window width
g_windowHeight = 900;          // Window height
```

## 📈 Performance

### Server
- **Tick Rate**: 60 Hz fixed timestep
- **CPU Usage**: ~5-10% (single core, 300 agents)
- **Memory**: ~50-100 MB
- **Network**: ~50 KB/s per connected client

### Client
- **Frame Rate**: Vsync (~60 FPS)
- **CPU Usage**: ~10-15% (rendering + simulation)
- **Memory**: ~100-150 MB
- **GPU**: Minimal (OpenGL 1.1 fixed pipeline)

## 🐛 Troubleshooting

### "Failed to initialize network manager"
- Check if port 27015 is available
- Run server as administrator
- Disable firewall temporarily to test

### "Connection to server failed"
- Verify server is running
- Check IP address and port
- Ensure no firewall blocking

### "Linker Error LNK2019: unresolved external symbol main"
- Ensure only one main() function is active
- Client uses main.cpp
- Server uses Server/ServerMain.cpp
- Don't compile both in same project

## 📝 Future Enhancements

- [ ] Save/Load game states
- [ ] Replay system
- [ ] More victory conditions (Science, Culture, Economic)
- [ ] Enhanced graphics (shaders, modern OpenGL)
- [ ] Procedural terrain generation
- [ ] More building types and unit classes
- [ ] Sound effects and music
- [ ] Scenario editor
- [ ] Tournament mode for AI evolution

## 📄 License

MIT License - Free to use, modify, and distribute

## 👥 Credits

Neural Network Civilization - A complex emergent strategy simulation
Developed with C++17, OpenGL, and Windows Sockets

---

**Enjoy watching civilizations evolve, compete, and cooperate in this dynamic simulation!**
