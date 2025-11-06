# C# Project Fixes (When Applicable)

## Critical Damage Multiplier Fix

If you have a critical damage multiplier that needs to be cast to int:

```csharp
// Before (may cause precision issues)
float criticalDamageMultiplier = 1.5f;
int damage = baseDamage * criticalDamageMultiplier;  // Implicit conversion

// After (explicit cast)
float criticalDamageMultiplier = 1.5f;
int damage = (int)(baseDamage * criticalDamageMultiplier);
```

## FactionId Missing Property Fix

If you're getting an error that FactionId is not added somewhere, ensure your PlayerData class includes the FactionId property:

```csharp
public class PlayerData
{
    public uint64 AccountId { get; set; }
    public float Health { get; set; }
    public float MaxHealth { get; set; }
    public int FactionId { get; set; }  // ADD THIS
    // ... other properties
}
```

And when creating KenshiCharacter:

```csharp
KenshiCharacter character = new KenshiCharacter
{
    CharacterId = playerId.GetHashCode(),
    PositionX = spawnPosition.X,
    PositionY = spawnPosition.Y,
    PositionZ = spawnPosition.Z,
    RotationX = spawnPosition.RotX,
    RotationY = spawnPosition.RotY,
    RotationZ = spawnPosition.RotZ,
    Health = playerData.Health,
    MaxHealth = playerData.MaxHealth,
    FactionId = playerData.FactionId  // This should now work
};
```

## For C++ Equivalent

In the current C++ project, the equivalent structures are in `DataStructures.h`:

```cpp
struct PlayerData {
    uint64_t accountId;
    char username[32];
    float health;
    float maxHealth;
    int factionId;  // Make sure this exists
    // ... other fields
};
```

If FactionId is missing, add it to the structure definition in `src/common/DataStructures.h`.
