#pragma once


UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA (DisplayName = "9mm"),
	EAT_AR UMETA (DisplayName = "AssaultRifle"),

	EAT_MAX UMETA (DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_SubmashineGun UMETA (DisplayName = "SubmashineGun"),
	EWT_AssaultRifle  UMETA (DisplayName = "AssaultRifle"),
	EWT_Pistol  UMETA (DisplayName = "Pistol"),

	EWT_MAX UMETA (DisplayName = "DefaultMAX"),
};