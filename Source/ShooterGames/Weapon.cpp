// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
    ThrowWeaponTime(0.7f),
    bFalling(false),
    Ammo(30),
    MagazineCapacity(30),
    WeaponType(EWeaponType::EWT_SubmashineGun),
    AmmoType(EAmmoType::EAT_9mm),
    ReloadMontageSection(FName(TEXT("Reload SMG"))),
    ClipBoneName(FName("smg_clip")),
    SlideDisplacement(0.0f),
    SlideDisplacementTime(0.1f),
    bMovingSlide(false),
    MaxSlideDisplacement(4.0f),
    MaxRecoilRotation(20.0f),
    bAutomatic(true)
{
    PrimaryActorTick.bCanEverTick = true;
    
    ItemType = EItemType::EIT_Weapon;
}

void AWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if(GetItemState() == EItemState::EIS_Falling && bFalling)
    {
        /**
         * Устанавливаем что оружие будет находиться в вертикальном положении
         * 
         */
        const FRotator MeshRotation {0.0f, GetItemMesh()->GetComponentRotation().Yaw, 0.0f};
        GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    }

    UpdateSlideDisplacement();
}
            
void AWeapon::BeginPlay()
{
    Super::BeginPlay();

    if(BoneToHide != FName(""))
    {
        GetItemMesh()->HideBoneByName(BoneToHide, EPhysBodyOp::PBO_None);
    }
}
void AWeapon::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    const FString WeaponTablePath {TEXT("DataTable'/Game/_Game/DataTables/DT_WeaponData.DT_WeaponData'")};

    UDataTable* WeaponTableObject = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *WeaponTablePath));

    if(WeaponTableObject)
    {
        FWeaponDataTable* WeaponDataRow = nullptr;
        switch(WeaponType)
        {
            case EWeaponType::EWT_SubmashineGun:
                WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("SubmachineGun"), TEXT(""));
            break;

            case EWeaponType::EWT_AssaultRifle:
                WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("AssaultRifle"), TEXT(""));
            break;
            case EWeaponType::EWT_Pistol:
                WeaponDataRow = WeaponTableObject->FindRow<FWeaponDataTable>(FName("Pistol"), TEXT(""));
            break;
        }

        if(WeaponDataRow)
        {
            AmmoType = WeaponDataRow->AmmoType;	
            Ammo = WeaponDataRow->WeaponAmmo;
            MagazineCapacity = WeaponDataRow->MagazinCapacity;
            
            SetPickupSound(WeaponDataRow->PickupSound);
            SetEquipSound(WeaponDataRow->EquipSound);
            GetItemMesh()->SetSkeletalMesh(WeaponDataRow->ItemMesh);
            SetItemName(WeaponDataRow->ItemName);
            SetItemIcon(WeaponDataRow->InventoryIcon);
            SetAmmoIcon(WeaponDataRow->AmmoIcon);
            SetClipBoneName(WeaponDataRow->ClipBoneName);
            SetMaterialInstance(WeaponDataRow->MaterialInstance);
           
            PreviousMaterialIndex = GetMaterialIndex();
            
            GetItemMesh()->SetMaterial(PreviousMaterialIndex, nullptr);
            SetMaterialIndex(WeaponDataRow->MaterialIndex);
            SetReloadMontageSection(WeaponDataRow->ReloadMontageSection);
            GetItemMesh()->SetAnimInstanceClass(WeaponDataRow->AnimBP);
            
            CrossHairsMiddle = WeaponDataRow->CrossHairsMiddle;
            CrossHairsLeft = WeaponDataRow->CrossHairsLeft;
            CrossHairsRight = WeaponDataRow->CrossHairsRight;
            CrossHairsBottom = WeaponDataRow->CrossHairsBottom;
            CrossHairsTop = WeaponDataRow->CrossHairsTop;

            AutoFireRate = WeaponDataRow->AutoFireRate;
            MuzzleFlash = WeaponDataRow->MuzzleFlash;
            FireSound = WeaponDataRow->FireSound;

            BoneToHide = WeaponDataRow->BoneToHide;

            bAutomatic = WeaponDataRow->bAutomatic;

            Damage = WeaponDataRow->Damage;
            HeadShotDamage = WeaponDataRow->HeadShotDamage;
            if(GetMaterialInstance())
			{
				SetDynamicMaterialInstance(UMaterialInstanceDynamic::Create(GetMaterialInstance() , this));
				GetDynamicMaterialInstance()->SetVectorParameterValue(TEXT("FresnelColor"), GetGlowColor());
				if(GetItemMesh())
				{
					GetItemMesh()->SetMaterial(GetMaterialIndex(), GetDynamicMaterialInstance());
					//GetItemMesh()->SetCustomDepthStencilValue(CustomDepthStencil);
				}
				EnableGlowMaterial();
			}
        }


    }
}

void AWeapon::ThrowWeapon()
{
    FRotator MeshRotation(0.0f, GetItemMesh()->GetComponentRotation().Yaw, 0.0f);
    GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
    
    const FVector MeshForward {GetItemMesh()->GetForwardVector()};
    FVector MeshRight {GetItemMesh()->GetRightVector()};
    /** Направление куда будет выбрасываться предмет */ 
    FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.0f, MeshForward);
    
    float RandomRotation {30.0f};
    ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.0f, 0.0f, 1.0f));
    ImpulseDirection *= 20'000.0f;
    GetItemMesh()->AddImpulse(ImpulseDirection);

    bFalling = true;
    GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
    EnableGlowMaterial();
}
void AWeapon::StopFalling()
{
    bFalling = false;
    SetItemState(EItemState::EIS_PickUp);
    StartPulseTimer();
}

void AWeapon::DecrementAmmo()
{
    if(Ammo - 1 <= 0)
    {
        Ammo = 0;
    }
    else
    {
        --Ammo;
    }
}

void AWeapon::ReloadAmmo(int32 Amount)
{
    Ammo += Amount; 
}

bool AWeapon::ClipIsFull()
{
    return Ammo >= MagazineCapacity;
}

void AWeapon::FinishMovingSlide()
{
    bMovingSlide = false;
}
void AWeapon::StartSlideTimer()
{
    bMovingSlide = true;
    GetWorldTimerManager().SetTimer(SlideTimer, this, &AWeapon::FinishMovingSlide, SlideDisplacementTime);
}

void AWeapon::UpdateSlideDisplacement()
{
    if(SlideDisplacementCurve && bMovingSlide)
    {
        const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(SlideTimer);
        const float CurveValue {SlideDisplacementCurve->GetFloatValue(ElapsedTime)};

        SlideDisplacement = CurveValue * MaxSlideDisplacement;
        RecoilRotation = CurveValue * MaxRecoilRotation;
    }
}