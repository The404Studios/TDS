# Multiplayer Testing Guide

Complete guide for testing multiplayer features in Extraction Shooter.

## Quick Test Setup

### Automated Testing (Easiest)

```cmd
# Run the quick launcher
run_game.bat

# Enter number of clients when prompted (e.g., 2 or 3)
```

This automatically starts the server and multiple clients for testing.

### Manual Testing

```cmd
# Terminal 1: Start server
x64\Release\ExtractionShooterServer.exe

# Terminal 2: Start client 1
x64\Release\ExtractionShooterRaylib.exe

# Terminal 3: Start client 2
x64\Release\ExtractionShooterRaylib.exe

# Terminal 4: Start client 3 (optional)
x64\Release\ExtractionShooterRaylib.exe
```

---

## Test Scenarios

### 1. Basic Connectivity Test

**Purpose**: Verify multiple clients can connect

**Steps**:
1. Start server
2. Launch 2 clients
3. On each client, create different accounts:
   - Client 1: Username "Player1", Password "test123"
   - Client 2: Username "Player2", Password "test123"
4. Both should successfully login

**Expected Result**: Both clients show main menu

**Server Console Output**:
```
[Server] Server started on port 7777
[AuthManager] Registered new account: Player1
[AuthManager] Player1 logged in successfully
[AuthManager] Registered new account: Player2
[AuthManager] Player2 logged in successfully
```

---

### 2. Lobby System Test

**Purpose**: Test party formation and matchmaking

**Steps**:
1. Login with 2+ clients
2. **Client 1** (Leader):
   - Click "Enter Raid"
   - Creates a lobby automatically
   - Press 'R' to ready up
3. **Client 2** (Teammate):
   - Would need lobby joining UI (currently auto-enters raid)
   - System creates separate lobbies

**Note**: Current implementation creates individual lobbies. Full party system would require lobby browser UI.

**Expected Behavior**:
- Each player can ready up independently
- Players spawn into separate raids
- Future: Players in same lobby spawn together

---

### 3. In-Raid Multiplayer Test

**Purpose**: See other players in the same match

**Current Implementation**:
- Players currently spawn in separate match instances
- To see each other, they need to be in same party/lobby (requires party UI)

**Testing Player Visibility** (Future feature):
1. Two players enter same match
2. Both should see each other as blue cubes (or models)
3. Name tags appear above other players
4. Movement should be synchronized

**Expected Sync**:
- Position updates: 10 Hz
- Smooth interpolation between positions
- Animations synchronized

---

### 4. Combat Test

**Purpose**: Test player vs player combat

**Steps**:
1. Two players in same raid
2. Player 1 shoots at Player 2
3. Check damage registration

**Server Validation**:
```
[MatchManager] Player <ID> took <damage> damage (HP: <remaining>)
```

**On Death**:
```
[MatchManager] Player <ID> died in match <matchID>
[MatchManager] Created corpse <corpseID> for player <ID>
```

**Client Response**:
- Victim sees red damage flash
- Health bar updates
- On death: "YOU DIED" screen

---

### 5. Corpse Looting Test

**Purpose**: Test body looting system

**Steps**:
1. Player 1 dies (take enough damage)
2. Corpse spawns at death location
3. Player 2 approaches corpse (within 3 meters)
4. Player 2 presses 'F' to loot

**Expected Server Output**:
```
[MatchManager] Player <ID> died in match <matchID>
[MatchManager] Created corpse <corpseID> for player <ID>
[MatchManager] Player <ID2> looted <item> from corpse <corpseID>
```

**Expected Client Behavior**:
- Corpse rendered as gray cube (or custom model)
- Gold indicator if corpse has loot
- Loot interface opens on 'F' press
- Items transfer to player inventory

---

### 6. Extraction Test

**Purpose**: Test extraction mechanics

**Steps**:
1. Player moves to extraction zone (green marker)
2. Gets within zone radius
3. Presses 'E' to extract
4. Wait for extraction timer

**Extraction Zones**:
- **Railroad Bridge**: (-120, 0, -120), radius 10m
- **Scav Checkpoint**: (130, 0, 130), radius 12m
- **Boat Dock**: (-100, 0, 140), radius 8m

**Expected Behavior**:
- Green "EXTRACTED!" screen
- Player keeps all collected loot
- Returns to main menu
- Loot added to stash

---

### 7. Network Stress Test

**Purpose**: Test server under load

**Setup**:
```cmd
# Run 4-8 clients simultaneously
run_game.bat
# Enter: 8
```

**Test Actions**:
- All clients login
- All enter raids
- Move around
- Shoot weapons
- Loot items

**Monitor**:
- Server CPU usage
- Network bandwidth
- Client FPS
- Server tick rate (should maintain 60 Hz)

**Expected Performance**:
- 60 FPS on clients with modern GPU
- <50ms network latency on LAN
- Server handles 8 clients smoothly

---

### 8. NAT Punchthrough Test (Advanced)

**Purpose**: Test P2P connectivity through NAT

**Requirements**:
- Two PCs on different networks
- Or VPN/tunneling software for testing

**Setup**:
```cmd
# On public server (or first PC)
NatPunchServer.exe 3478

# On game server
ExtractionShooterServer.exe

# On client PCs
ExtractionShooterRaylib.exe --nat --server <public_server_ip>
```

**Expected Behavior**:
- Clients register with NAT server
- NAT server facilitates UDP hole punching
- Clients can establish direct P2P connections
- Lower latency voice chat (future feature)

**NAT Server Output**:
```
[NatServer] NAT Punchthrough Server started on UDP port 3478
[NatServer] Registered client: Player_1234 from <IP>:<PORT>
[NatServer] Facilitating punch: target Player_5678
```

---

## Common Issues & Solutions

### Issue: "Failed to connect to server"
**Solution**:
- Ensure server is running first
- Check Windows Firewall (allow port 7777)
- Verify server console shows "Server started on port 7777"

### Issue: "Players can't see each other"
**Solution**:
- Currently expected - requires same lobby/party
- Party system needs lobby browser UI
- Future feature: lobby joining

### Issue: "High latency/lag"
**Solution**:
- Check network quality (ping)
- Reduce number of clients
- Ensure server has adequate CPU
- Use wired connection instead of WiFi

### Issue: "Corpse not appearing"
**Solution**:
- Check server console for corpse creation message
- Ensure player died (health reached 0)
- Client might need corpse spawn packet (networking)

### Issue: "Can't loot corpse"
**Solution**:
- Get closer (must be within 3 meters)
- Check if corpse has items
- Verify player is alive

---

## Performance Benchmarks

### Expected Performance (Release Build)

| Clients | Server CPU | Server RAM | Client FPS | Latency |
|---------|-----------|------------|------------|---------|
| 1 | <5% | ~50 MB | 60+ | <1ms |
| 2 | <10% | ~60 MB | 60+ | <5ms |
| 4 | <20% | ~80 MB | 60+ | <10ms |
| 8 | <40% | ~120 MB | 60+ | <20ms |

*Tested on: i5-8400, 16GB RAM, GTX 1060*

### Client Performance

**Minimum Specs**:
- CPU: i3 or equivalent
- GPU: Integrated graphics
- RAM: 4 GB
- Expected FPS: 30-60

**Recommended Specs**:
- CPU: i5 or equivalent
- GPU: GTX 1050 / RX 560
- RAM: 8 GB
- Expected FPS: 60+

---

## Advanced Testing

### Packet Inspection

Use Wireshark to inspect network traffic:
1. Filter: `tcp.port == 7777`
2. Observe packet structure
3. Verify encryption (future)

### Server Load Testing

Simulate many concurrent users:
```python
# Python script to spawn multiple test connections
import socket
import threading

def test_connection(client_id):
    sock = socket.socket()
    sock.connect(('127.0.0.1', 7777))
    # Send login packet
    # ...

for i in range(50):
    threading.Thread(target=test_connection, args=(i,)).start()
```

### Database Stress Test

1. Create many accounts rapidly
2. Verify persistence
3. Check for corruption
4. Test concurrent writes

---

## Automated Test Script (Future)

**Planned Features**:
- Automated login sequences
- Scripted player movements
- Automated combat scenarios
- Regression testing

**Example**:
```python
# test_multiplayer.py (conceptual)
def test_basic_connectivity():
    server = start_server()
    client1 = start_client()
    client2 = start_client()

    assert client1.login("test1", "pass")
    assert client2.login("test2", "pass")
    assert client1.is_connected()
    assert client2.is_connected()
```

---

## Reporting Issues

When reporting multiplayer bugs, include:

1. **Server Console Output** (last 50 lines)
2. **Number of clients connected**
3. **Steps to reproduce**
4. **Expected vs actual behavior**
5. **Network conditions** (LAN, WiFi, internet)
6. **System specs** (CPU, RAM, GPU)

**Example Bug Report**:
```
Title: Players can't see each other in raids

Description:
- 2 clients connected to server
- Both entered raids
- Both in same map
- Neither client renders other player

Server Output:
[MatchManager] Match created: 1 (Map: Default, Players: 1)
[MatchManager] Match created: 2 (Map: Default, Players: 1)

Expected: Both in same match
Actual: Separate matches created
```

---

## Next Steps

After basic testing works:

1. **Party System**: Implement lobby browser UI
2. **Voice Chat**: Integrate VOIP using NAT punchthrough
3. **Anti-Cheat**: Enable all validation in production
4. **Leaderboards**: Add global player rankings
5. **Dedicated Servers**: Deploy to cloud hosting

Happy testing! 🎮
