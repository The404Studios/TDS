# Deployment Guide

Complete guide for deploying Extraction Shooter to production servers.

## Table of Contents

1. [Local Deployment](#local-deployment)
2. [LAN Deployment](#lan-deployment)
3. [Public Server Deployment](#public-server-deployment)
4. [Cloud Hosting](#cloud-hosting)
5. [Security Considerations](#security-considerations)
6. [Monitoring & Maintenance](#monitoring--maintenance)

---

## Local Deployment

### For Personal Testing

**Requirements**:
- Windows 10/11
- Visual Studio 2022 (for building)
- 4 GB RAM minimum
- Port 7777 available

**Steps**:

1. **Build the project**:
   ```cmd
   setup_and_run.bat
   ```

2. **Run locally**:
   ```cmd
   run_game.bat
   ```

3. **Configure server** (optional):
   - Edit `server_config.ini`
   - Adjust match settings, loot counts, etc.

**Default Configuration**:
- Server IP: 127.0.0.1 (localhost)
- Port: 7777
- Max players: 100
- Data storage: `Data/` directory

---

## LAN Deployment

### For Local Network Play (Home/Office)

**Requirements**:
- Windows Server 2019+ or Windows 10/11
- Static local IP recommended
- Router admin access (for port forwarding)

### Setup Steps

#### 1. Configure Server PC

```cmd
# Build server
msbuild ExtractionShooterServer.vcxproj /p:Configuration=Release /p:Platform=x64

# Note your local IP
ipconfig
# Look for "IPv4 Address" (e.g., 192.168.1.100)
```

#### 2. Configure Firewall

**Windows Firewall**:
```cmd
# Allow inbound on port 7777
netsh advfirewall firewall add rule name="Extraction Shooter Server" dir=in action=allow protocol=TCP localport=7777

# Verify rule
netsh advfirewall firewall show rule name="Extraction Shooter Server"
```

**Or use GUI**:
1. Windows Defender Firewall → Advanced Settings
2. Inbound Rules → New Rule
3. Port → TCP → 7777
4. Allow the connection
5. Name: "Extraction Shooter Server"

#### 3. Start Server

```cmd
cd x64\Release
ExtractionShooterServer.exe
```

#### 4. Connect Clients (LAN)

On client PCs:
```cmd
# Edit client connection (or use command line arg when implemented)
ExtractionShooterRaylib.exe --server 192.168.1.100
```

Or modify `src/client/RaylibMain.cpp`:
```cpp
const char* GAME_SERVER_IP = "192.168.1.100";  // Your server IP
```

---

## Public Server Deployment

### For Internet-Accessible Server

**Requirements**:
- Dedicated server or VPS
- Public IP address
- Domain name (optional but recommended)
- SSL certificate (for future HTTPS)

### Hosting Options

#### Option A: Dedicated Home Server

**Requirements**:
- High-speed internet (10+ Mbps upload)
- Static IP or DDNS service
- Router with port forwarding

**Steps**:

1. **Set up port forwarding** on your router:
   - External Port: 7777
   - Internal Port: 7777
   - Internal IP: Server PC's local IP
   - Protocol: TCP

2. **Use Dynamic DNS** (if no static IP):
   - Services: No-IP, DuckDNS, DynDNS
   - Create hostname (e.g., `myserver.ddns.net`)
   - Install DDNS client on server

3. **Start server**:
   ```cmd
   x64\Release\ExtractionShooterServer.exe
   ```

4. **Share connection info**:
   - External IP or DDNS hostname
   - Port: 7777

#### Option B: Cloud VPS (Recommended)

**Popular Providers**:
- **AWS EC2**: Scalable, expensive
- **Google Cloud**: Good free tier
- **DigitalOcean**: Simple, affordable ($5-20/mo)
- **Vultr**: Gaming-optimized
- **OVH Game Servers**: Dedicated game hosting

**Recommended Specs**:
| Players | CPU | RAM | Storage | Bandwidth |
|---------|-----|-----|---------|-----------|
| 1-10 | 1 core | 2 GB | 20 GB | 1 TB/mo |
| 10-25 | 2 cores | 4 GB | 40 GB | 2 TB/mo |
| 25-50 | 4 cores | 8 GB | 60 GB | 4 TB/mo |
| 50-100 | 8 cores | 16 GB | 100 GB | 8 TB/mo |

---

## Cloud Hosting

### AWS EC2 Deployment

#### 1. Launch Instance

```bash
# Instance type: t3.medium (2 vCPU, 4 GB RAM)
# OS: Windows Server 2019/2022
# Security Group: Allow TCP 7777
```

#### 2. Connect to Instance

- Use RDP to connect
- Install Visual Studio Build Tools or full VS

#### 3. Deploy Server

```powershell
# Upload built executable via RDP or:
scp -i key.pem x64/Release/ExtractionShooterServer.exe Administrator@<public-ip>:C:\Server\

# Or build on server
git clone <your-repo>
cd TDS
.\setup_and_run.bat
```

#### 4. Configure as Windows Service

```powershell
# Install NSSM (Non-Sucking Service Manager)
choco install nssm

# Create service
nssm install ExtractionShooter "C:\Server\ExtractionShooterServer.exe"
nssm set ExtractionShooter AppDirectory "C:\Server"
nssm set ExtractionShooter Start SERVICE_AUTO_START

# Start service
nssm start ExtractionShooter
```

### DigitalOcean Deployment (Simple)

#### 1. Create Droplet

```bash
# Choose:
# - Windows Server 2019
# - 2 GB RAM ($10/mo)
# - Region: Nearest to players
# - Firewall: TCP 7777
```

#### 2. Deploy via PowerShell

```powershell
# After RDP connection
# Clone repo
git clone https://github.com/YourUsername/TDS.git
cd TDS

# Download dependencies
.\download_dependencies.bat

# Build
msbuild ExtractionShooter.sln /p:Configuration=Release /p:Platform=x64

# Run
cd x64\Release
.\ExtractionShooterServer.exe
```

#### 3. Keep Server Running

Use `screen` or Windows Task Scheduler to run server on startup.

---

## Security Considerations

### Essential Security Measures

#### 1. Firewall Configuration

**Allow only necessary ports**:
```cmd
# Block all, allow specific
netsh advfirewall set allprofiles firewallpolicy blockinbound,blockoutbound

# Allow game server
netsh advfirewall firewall add rule name="Game Server" dir=in action=allow protocol=TCP localport=7777

# Allow RDP (if needed)
netsh advfirewall firewall add rule name="RDP" dir=in action=allow protocol=TCP localport=3389
```

#### 2. Account Security

**Edit server config**:
```ini
[AntiCheat]
ValidatePositions=true
ValidateProximity=true
DisableAntiCheat=false  # NEVER enable in production

[Debug]
EnableDebugCommands=false
AllowItemSpawn=false
DevMode=false  # CRITICAL: Always false in production
```

#### 3. Rate Limiting

**Prevent DDoS** (future implementation):
```ini
[Network]
MaxConnectionsPerIP=3
ConnectionTimeout=30
PacketRateLimit=100  # packets/second
```

#### 4. Data Protection

**Backup player data**:
```cmd
# Automated backup script
xcopy /E /I /Y Data\ Backups\Data_%date:~-4,4%%date:~-10,2%%date:~-7,2%\
```

**Schedule backups** (Task Scheduler):
- Daily: Full data backup
- Hourly: Incremental backup

#### 5. Admin Access

**Secure RDP**:
- Use strong passwords (20+ characters)
- Enable Network Level Authentication
- Use VPN for admin access
- Change default RDP port from 3389

#### 6. Monitoring

**Install monitoring tools**:
- **Task Manager**: Monitor CPU/RAM
- **Resource Monitor**: Track network usage
- **Event Viewer**: Check for errors
- **Custom logging**: Enable in `server_config.ini`

---

## Performance Optimization

### Server Optimization

**server_config.ini**:
```ini
[Performance]
TickRate=60  # Reduce to 30 for lower CPU usage
VerboseLogging=false  # Disable in production
AutoSaveInterval=600  # Increase to reduce I/O
```

**Windows Optimizations**:
```cmd
# Disable unnecessary services
services.msc
# Disable: Print Spooler, Xbox services, etc.

# Set high priority
wmic process where name="ExtractionShooterServer.exe" CALL setpriority "high priority"

# Disable Windows Defender real-time (if using other antivirus)
Set-MpPreference -DisableRealtimeMonitoring $true
```

### Network Optimization

**Quality of Service (QoS)**:
```cmd
# Prioritize game server traffic
netsh interface tcp set global autotuninglevel=normal
netsh interface tcp set global chimney=enabled
netsh interface tcp set global dca=enabled
netsh interface tcp set global netdma=enabled
```

**Bandwidth Management**:
- Reserve bandwidth for game server
- Limit other applications
- Use wired connection (not WiFi)

---

## Monitoring & Maintenance

### Health Checks

**Automated monitoring script** (PowerShell):
```powershell
# check_server.ps1
while($true) {
    $process = Get-Process -Name ExtractionShooterServer -ErrorAction SilentlyContinue

    if (!$process) {
        Write-Host "Server crashed! Restarting..."
        Start-Process "C:\Server\ExtractionShooterServer.exe"
    }

    Start-Sleep -Seconds 60
}
```

**Run as scheduled task**:
```cmd
schtasks /create /tn "ServerMonitor" /tr "powershell.exe -File C:\Server\check_server.ps1" /sc onstart /ru SYSTEM
```

### Log Rotation

**Prevent logs from filling disk**:
```powershell
# rotate_logs.ps1
$logPath = "C:\Server\server.log"
$maxSize = 50MB

if ((Get-Item $logPath).length -gt $maxSize) {
    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    Move-Item $logPath "C:\Server\Logs\server_$timestamp.log"
}
```

### Metrics to Monitor

**Key Performance Indicators**:
- CPU usage (target: <70%)
- RAM usage (target: <80%)
- Network latency (target: <50ms)
- Tick rate (target: 60 Hz)
- Player count
- Crash frequency

**Tools**:
- **Grafana**: Visual dashboards
- **Prometheus**: Metrics collection
- **Discord webhooks**: Alerts

---

## Backup & Recovery

### Backup Strategy

**What to backup**:
- `Data/` directory (player accounts, stashes)
- `server_config.ini`
- Server logs
- Build artifacts (optional)

**Backup script**:
```cmd
@echo off
set TIMESTAMP=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%
set BACKUP_DIR=E:\Backups\%TIMESTAMP%

mkdir %BACKUP_DIR%
xcopy /E /I /Y Data\ %BACKUP_DIR%\Data\
copy server_config.ini %BACKUP_DIR%\
copy server.log %BACKUP_DIR%\

echo Backup complete: %BACKUP_DIR%
```

**Automated backups**:
- Schedule: Every 6 hours
- Retention: Keep 7 days
- Off-site: Upload to cloud storage

### Disaster Recovery

**Recovery steps**:
1. Stop server
2. Restore `Data/` from backup
3. Verify data integrity
4. Restart server
5. Monitor logs for errors

**Recovery Time Objective (RTO)**: <1 hour
**Recovery Point Objective (RPO)**: <6 hours

---

## Scaling

### Horizontal Scaling (Multiple Servers)

**Load balancer setup**:
```
[Load Balancer] → [Game Server 1] (50 players)
                → [Game Server 2] (50 players)
                → [Game Server 3] (50 players)
```

**Shared database**:
- Migrate to MySQL/PostgreSQL
- All servers connect to same DB
- Redis for session management

### Vertical Scaling (More Resources)

**When to scale up**:
- CPU usage >80%
- RAM usage >80%
- Tick rate drops below 60 Hz
- Player complaints about lag

**Upgrade path**:
- 2 CPU → 4 CPU
- 4 GB RAM → 8 GB RAM
- 1 Gbps NIC → 10 Gbps NIC

---

## Troubleshooting

### Common Issues

**Server won't start**:
```cmd
# Check port availability
netstat -ano | findstr :7777

# Kill process using port
taskkill /PID <PID> /F

# Check firewall
netsh advfirewall firewall show rule name=all | findstr 7777
```

**High CPU usage**:
- Reduce tick rate
- Optimize match logic
- Check for infinite loops in logs

**Players can't connect**:
- Verify port forwarding
- Check firewall rules
- Test from external IP: `telnet <ip> 7777`

**Frequent crashes**:
- Check server logs
- Update to latest build
- Add exception handling
- Monitor memory leaks

---

## Production Checklist

Before going live:

- [ ] Build in Release mode
- [ ] Disable all debug options
- [ ] Enable anti-cheat
- [ ] Configure automated backups
- [ ] Set up monitoring
- [ ] Document admin procedures
- [ ] Test disaster recovery
- [ ] Configure firewall properly
- [ ] Use strong admin passwords
- [ ] Plan scaling strategy

---

## Support & Community

**Admin Resources**:
- Server admin Discord
- Issue tracker
- Documentation wiki
- Community forums

**Getting Help**:
1. Check logs first
2. Search existing issues
3. Provide full error details
4. Include server specs & config

---

## Cost Estimate

**Monthly Hosting Costs**:

| Players | Hosting | Bandwidth | Total |
|---------|---------|-----------|-------|
| 10 | $5 | Included | **$5** |
| 25 | $10 | $5 | **$15** |
| 50 | $20 | $10 | **$30** |
| 100 | $40 | $20 | **$60** |

*Estimates for VPS hosting (DigitalOcean, Vultr)*

---

**Need help deploying? Check the troubleshooting section or ask in community Discord!**
