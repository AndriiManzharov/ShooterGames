// Fill out your copyright notice in the Description page of Project Settings.

#include "Ammo.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Item.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "ShooterCharacter.h"
#include "Sound/SoundCue.h"
#include "Weapon.h"
#include "Components/CapsuleComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ShooterGames.h"
#include "BulletHitInterface.h"
#include "Enemy.h"
#include "EnemyController.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter():
	BaseTurnRate(45.0f),
	BaseLookUpRate(45.0f),
	bAiming(false),
	CameraDefaultFOV(0.0f),
	CameraZoomedFOV(60.0f),
	CameraZoomedSpeedFOV(50.0f),
	HipTurnRate(1.0f),
	HipLookUpRate(1.0f),
	AimingTurnRate(0.2f),
	AimingLookUpRate(0.2f),
	CrossHairSpreadMultiplier(0.0f),
	CrossHairVelocityFactor(0.0f),
	CrossHairInAirFactor(0.0f),
	CrossHairAimFactor(0.0f),
	CrossHairShootingFactor(0.0f),
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	bShouldFire(true),
	bFireButtonPressed(false),
	bShoudTraceForItems(false),
	CameraInterpDistance(250.0f),
	CameraInterpElevation(65.0f),
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false),
	BaseMovementSpeed(650.0f),
	CrouchMovementSpeed(300.0f),
	StandingCapsuleHalfHight(88.0f),
	CrouchingCapsuleHalfHight(44.0f),
	BaseGroundFriction(2.0f),
	CrouchGroundFriction(100.0f),
	bAimingButtonPressed(false),
	bShouldPlayPickupSound(true),
	bShouldPlayEquipSound(true),
	PickupSoundResetTime(0.2f),
	EquipSoundResetTime(0.2f),
	HighlightedSlot(-1),
	Health(100.0f),
	MaxHealth(100.0f),
	StunChance(0.25),
	bDead(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector {0.0f, 50.0f, 50.0f};
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	/** Вращаем только камеру но не меш*/
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	/**
	 * Персонаж двигается в ту сторону куда ориентирован с уровнем указаном в 
	 * GetCharacterMovement()->RotationRate
	 * */
	GetCharacterMovement()->bOrientRotationToMovement = false;
	/** уровень для bOrientRotationToMovement смотрим только по сторонам
	 * те ось Y */ 
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>("HandSceneComponent");

	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponInterpComp"));
	WeaponInterpComp->SetupAttachment(GetCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp1"));
	InterpComp1->SetupAttachment(GetCamera());

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp2"));
	InterpComp2->SetupAttachment(GetCamera());

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp3"));
	InterpComp3->SetupAttachment(GetCamera());

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp4"));
	InterpComp4->SetupAttachment(GetCamera());

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp5"));
	InterpComp5->SetupAttachment(GetCamera());

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("InterpComp6"));
	InterpComp6->SetupAttachment(GetCamera());
	
}


// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if(Camera)
	{
		CameraDefaultFOV = Camera->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	EquipWeapon(SpawnDefaultWeapon());
	Inventory.Add(EquippedWeapon);
	EquippedWeapon->SetSlotIndex(0);
	EquippedWeapon->DisableCustomDepth();
	EquippedWeapon->DisableGlowMaterial();
	EquippedWeapon->SetCharacter(this);
	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	InitializeInterpLocations();
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraIterpZoom(DeltaTime);
	SetLookRates();

	CalculateCrossHairSpread(DeltaTime);

	TraceForItem();

	InterpCapsuleHalfHight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("TurnAtRate", this, &AShooterCharacter::TurnAtRate);
	
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction("AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);

	PlayerInputComponent->BindAction("Select", IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction("Select", IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction("ReloadButton", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this, &AShooterCharacter::FKeyPressed);

	PlayerInputComponent->BindAction("1Key", IE_Pressed, this, &AShooterCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this, &AShooterCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this, &AShooterCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this, &AShooterCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this, &AShooterCharacter::FiveKeyPressed);

	
}

void AShooterCharacter::MoveForward(float Amount)
{
	if((Controller != nullptr) && (!FMath::IsNearlyEqual(Amount, 0.0f)))
	{
		const FRotator Rotation {Controller->GetControlRotation()};
		const FRotator YawRotation {0.0f, Rotation.Yaw, 0.0f};

		const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Amount);
	}
}

void AShooterCharacter::MoveRight(float Amount)
{
	if((Controller != nullptr) && (!FMath::IsNearlyEqual(Amount, 0.0f)))
	{
		const FRotator Rotation {Controller->GetControlRotation()};
		const FRotator YawRotation {0.0f, Rotation.Yaw, 0.0f};

		const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Amount);
	}
}

void AShooterCharacter::LookUpRate(float Amount)
{
	AddControllerPitchInput(Amount * BaseLookUpRate);	
}

void AShooterCharacter::TurnAtRate(float Amount)
{
	AddControllerYawInput(Amount * BaseTurnRate);	
}

void AShooterCharacter::FireWeapon()
{
	if(!EquippedWeapon) return;

	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if(WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		
		StartCrosshairBulletFire();
		
		EquippedWeapon->DecrementAmmo();

		StartFireTimer();

		if(EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol)
		{
			EquippedWeapon->StartSlideTimer();
		}
	}

	




}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FHitResult& OutHitResult)
{
	FHitResult CrosshairHitResult;
	FVector OutBeamLocation;
	bool bCrossHairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);


	/**
	 * Вторая проверка на пересечение с объектами 
	 * игры теперь начало линии от оружия
	 */

	const FVector WeaponTraceStart {MuzzleSocketLocation};
	const FVector StartToEnd {OutBeamLocation - MuzzleSocketLocation};
	const FVector WeaponTraceEnd {MuzzleSocketLocation + StartToEnd * 1.25f};
	GetWorld()->LineTraceSingleByChannel(OutHitResult, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);	

	if(!OutHitResult.bBlockingHit)
	{
		OutHitResult.Location = OutBeamLocation;
		return false;
	}

	return true;
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;
	if(CombatState != ECombatState::ECS_Reloading && 
	   CombatState != ECombatState::ECS_Equipping && 
	   CombatState != ECombatState::ECS_Stunned)
	{
		Aim();
	}
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;
	StopAim();
}

void AShooterCharacter::CameraIterpZoom(float DeltaTime)
{
	CameraCurrentFOV = bAiming ? 
	FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, CameraZoomedSpeedFOV) :
	FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, CameraZoomedSpeedFOV);
	
	Camera->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::SetLookRates()
{
	if(bAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrossHairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange {0.0f, 600.0f};
	FVector2D VelocityMultiplerRange {0.0f, 1.0f};
	FVector Velocity {GetVelocity()};
	Velocity.Z = 0;

	CrossHairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplerRange, Velocity.Size());

	if(GetCharacterMovement()->IsFalling())
	{
		CrossHairInAirFactor = FMath::FInterpTo(CrossHairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrossHairInAirFactor = FMath::FInterpTo(CrossHairInAirFactor, 0.0f, DeltaTime, 30.0f);
	}

	if(bAiming)
	{
		CrossHairAimFactor = FMath::FInterpTo(CrossHairAimFactor, 0.6f, DeltaTime, 30.0f);
	}
	else
	{
		CrossHairAimFactor = FMath::FInterpTo(CrossHairAimFactor, 0.0f, DeltaTime, 30.0f);
	}

	if(bFiringBullet)
	{
		CrossHairShootingFactor = FMath::FInterpTo(CrossHairShootingFactor, 0.3f, DeltaTime, 60.0f);
	}
	else
	{
		CrossHairShootingFactor = FMath::FInterpTo(CrossHairShootingFactor, 0.0f, DeltaTime, 60.0f);
	}

	CrossHairSpreadMultiplier = 0.5 + CrossHairVelocityFactor + CrossHairInAirFactor - CrossHairAimFactor + CrossHairShootingFactor;
}

float AShooterCharacter::GetCrossHairSpreadMultiplier() const
{
	return CrossHairSpreadMultiplier;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrossHairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	StartFireTimer();
}
void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	if(EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_FireTimerInProgress;
	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, EquippedWeapon->GetAutoFireRate());
}


void AShooterCharacter::AutoFireReset()
{
	if(CombatState == ECombatState::ECS_Stunned) return;

	CombatState = ECombatState::ECS_Unoccupied;
	if(WeaponHasAmmo())
	{
		if(bFireButtonPressed && EquippedWeapon->GetAutomatic())
		{
			FireWeapon();
		}
	}else
	{
		ReloadWeapon();
	}
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);

	}

	FVector2D CrossHairLocation {ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f};
	
	FVector CrossHairWorldPosition;
	FVector CrossHairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), 
								CrossHairLocation, 
								CrossHairWorldPosition,
								CrossHairWorldDirection);
	
	if(bScreenToWorld)
	{
		const FVector Start{CrossHairWorldPosition};
		const FVector End{Start + CrossHairWorldDirection * 50'000.0f};
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if(OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

void AShooterCharacter::IncrementOverlappedItemCount(int8 Amount)
{
	if(OverlappedItemCounts + Amount <= 0)
	{
		OverlappedItemCounts = 0;
		bShoudTraceForItems = false;
	}
	else
	{
		OverlappedItemCounts += Amount;
		bShoudTraceForItems = true; 
	}
}

void AShooterCharacter::TraceForItem()
{
	if(bShoudTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		TraceUnderCrosshairs(ItemTraceResult, HitLocation);
		if(ItemTraceResult.bBlockingHit)
		{
			TraceHitItem = Cast<AItem>(ItemTraceResult.Actor);
			const auto TraceHitWeapon = Cast<AWeapon>(TraceHitItem);
			if(TraceHitWeapon)
			{
				if(HighlightedSlot == -1)
				{
					HighlightInventorySlot();
				}
			}
			else
			{
				if(HighlightedSlot != -1)
				{
					UnHighlightInventorySlot();
				}
			}
			if(TraceHitItem && TraceHitItem->GetItemState() == EItemState::EIS_EquipInterping)
			{
				TraceHitItem = nullptr;
			}
			
			if(TraceHitItem && TraceHitItem->GetPickupWidget())
			{
				TraceHitItem->GetPickupWidget()->SetVisibility(true);
				TraceHitItem->EnableCustomDepth();
				if(Inventory.Num() >= INVENTORY_CAPACITY)
				{
					TraceHitItem->SetCharacterInventoryFull(true);
				}
				else
				{
					TraceHitItem->SetCharacterInventoryFull(false);
				}
			}

			if(TraceHitItemLastFrame)
			{
				if (TraceHitItem != TraceHitItemLastFrame)
				{
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
					TraceHitItemLastFrame->DisableCustomDepth();
				}
			}
			TraceHitItemLastFrame = TraceHitItem;
		}
	}
	/** Больше не пересекаем сферы предметов*/
	else if(TraceHitItemLastFrame && TraceHitItem)
	{
		TraceHitItemLastFrame->GetPickupWidget()->SetVisibility(false);
		TraceHitItem->DisableCustomDepth();
	}

}
AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if(DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* WeaponToEquip, bool bSwapping)
{
	if(WeaponToEquip)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if(HandSocket)
		{
			HandSocket->AttachActor(WeaponToEquip, GetMesh());
		}

		if(EquippedWeapon == nullptr)
		{
			// -1 Нет текущего оружия не будет анимации
			EquipItemDelegate.Broadcast(-1, WeaponToEquip->GetSlotIndex());
		}
		else if(!bSwapping)
		{
			EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), WeaponToEquip->GetSlotIndex());
		}

		EquippedWeapon = WeaponToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if(EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentTransformRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	if(TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this, true);
		TraceHitItem = nullptr;
	}
	
}

void AShooterCharacter::SelectButtonReleased()
{
	//
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	if(Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
	{
		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
	}
	
	DropWeapon();
	EquipWeapon(WeaponToSwap, true);
	TraceHitItem = nullptr;
	TraceHitItemLastFrame = nullptr;
}

FVector AShooterCharacter::GetCameraInterpLocation()
{
	const FVector CameraWorldLocation {Camera->GetComponentLocation()};
	const FVector CameraForward { Camera->GetForwardVector()};

	return CameraWorldLocation + CameraForward * CameraInterpDistance + 
	FVector (0.0f, 0.0f, CameraInterpElevation);
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();

	auto Weapon = Cast<AWeapon>(Item);
	if(Weapon)
	{
		if(Inventory.Num() < INVENTORY_CAPACITY)
		{
			Weapon->SetSlotIndex(Inventory.Num());
			Inventory.Add(Weapon);
			Weapon->SetItemState(EItemState::EIS_PickedUp);
		}
		else
		{
			SwapWeapon(Weapon);
		}
	}

	auto Ammo = Cast<AAmmo>(Item);
	if(Ammo)
	{
		PickupAmmo(Ammo);
	}
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if(!EquippedWeapon)
		return false;
	return EquippedWeapon->GetAmmo() > 0;
}


void AShooterCharacter::PlayFireSound()
{
	if(EquippedWeapon->GetFireSound())
	{
		UGameplayStatics::PlaySound2D(this, EquippedWeapon->GetFireSound());
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");
	if(BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());
		if(EquippedWeapon->GetMuzzleFlash())
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EquippedWeapon->GetMuzzleFlash(), SocketTransform);
		}
		FHitResult BeamHitResult;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamHitResult);

		if(bBeamEnd)
		{
			/**
			 * Проверяем реализует ли Actor - BulletHitInterface
			 * 
			 */

			if(BeamHitResult.Actor.IsValid())
			{
				/**
				 * Если указатель валидный каст будет успешный в том
				 * случае если объект наследован от  IBulletHitInterface
				 * 
				 */
				IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(BeamHitResult.Actor.Get());
				if(BulletHitInterface)
				{
					BulletHitInterface->BulletHit_Implementation(BeamHitResult, this, GetController());
				}

				AEnemy* HitEnemy = Cast<AEnemy>(BeamHitResult.Actor.Get());
				if(HitEnemy)
				{
					float Damage {};
					bool bHeadShot{};
					if(BeamHitResult.BoneName.ToString() == HitEnemy->GetHeadBoneName())
					{
						Damage = EquippedWeapon->GetHeadShotDamage();
						bHeadShot = true;
					}
					else
					{
						Damage = EquippedWeapon->GetDamage();
						bHeadShot = false;
					}
					UGameplayStatics::ApplyDamage(BeamHitResult.Actor.Get(),
						Damage,
						GetController(),
						this,
						UDamageType::StaticClass());
					
					HitEnemy->ShowHitNumber(Damage, BeamHitResult.Location, bHeadShot);
				}
			}
			else
			{
				/**
				 * Делаем спавн частиц по умолчинию
				 * 
				 */
				if(ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, BeamHitResult.Location);
				}
			}


			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);

			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamHitResult.Location);
			}
		}		
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
		
	UAnimInstance* AnimInstace = GetMesh()->GetAnimInstance();

	if(AnimInstace && HipFireMontage)
	{
		AnimInstace->Montage_Play(HipFireMontage);
		AnimInstace->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	
	if(EquippedWeapon == nullptr) return;
	if(CarryingAmmo() && !EquippedWeapon->ClipIsFull())
	{
		if(bAiming)
		{
			StopAim();
		}
		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(ReloadMontage && AnimInstance)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquippedWeapon->GetReloadMontageSection());
		}
	}
}

void AShooterCharacter::FinishReloading()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	CombatState = ECombatState::ECS_Unoccupied;

	if(bAiming)
	{
		Aim();
	}

	if(!EquippedWeapon) return;

	const auto AmmoType {EquippedWeapon->GetAmmoType()};

	if(AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];
		const int32 MagEmptySpace = EquippedWeapon->GeMagazineCapacity() - 
									EquippedWeapon->GetAmmo();

		if(MagEmptySpace > CarriedAmmo)
		{
			EquippedWeapon->ReloadAmmo(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			EquippedWeapon->ReloadAmmo(MagEmptySpace);
			CarriedAmmo -= MagEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if(!EquippedWeapon) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if(AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if(EquippedWeapon == nullptr) return;
	if(HandSceneComponent == nullptr) return;
	int32 ClipBoneIndex {EquippedWeapon->GetItemMesh()->GetBoneIndex(EquippedWeapon->GetClipBoneName())};
	ClipTransform = EquippedWeapon->GetItemMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules (EAttachmentRule::KeepRelative, true);

	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);
	EquippedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReplaceClip()
{
	EquippedWeapon->SetMovingClip(false	);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if(!GetMovementComponent()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	GetCharacterMovement()->MaxWalkSpeed = bCrouching ? 
											CrouchMovementSpeed : 
											BaseMovementSpeed;
	GetCharacterMovement()->GroundFriction = bCrouching ?
											  CrouchGroundFriction :
											  BaseGroundFriction;
}

void AShooterCharacter::Jump() 
{
	if(bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
}

void AShooterCharacter::InterpCapsuleHalfHight(float DeltaTime)
{
	float TargetCapsuleHalfHight {};
	if(bCrouching)
	{
		TargetCapsuleHalfHight = CrouchingCapsuleHalfHight;
	}
	else
	{
		TargetCapsuleHalfHight = StandingCapsuleHalfHight;
	}

	const float InterpHalfHight {FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHight, DeltaTime, 20.f)};
	
	/**
	 * @brief Отрицательное значение, если персонаж приседает,
	 * 		  Положительное значение, если персонаж встаёт
	 * 
	 */
	const float DeltaCapsuleHalfHight {InterpHalfHight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	
	const FVector MeshOffest {0.0f, 0.0f, -DeltaCapsuleHalfHight};

	GetMesh()->AddLocalOffset(MeshOffest);

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHight);
}
void AShooterCharacter::Aim()
{
	bAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

void AShooterCharacter::StopAim()
{
	bAiming = false;
	if(!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	if(AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount {AmmoMap[Ammo->GetAmmoType()]};
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if(EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		if(EquippedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if(Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	/**
	 * @brief 
	 * 
	 * @return Возвращает пустую структуру с неинициализированными полями
	 * 		 
	 */
	return FInterpLocation();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation {WeaponInterpComp, 0};
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1 {InterpComp1, 0};	
	InterpLocations.Add(InterpLoc1);
	
	FInterpLocation InterpLoc2 {InterpComp2, 0};	
	InterpLocations.Add(InterpLoc2);
	
	FInterpLocation InterpLoc3 {InterpComp3, 0};	
	InterpLocations.Add(InterpLoc3);
	
	FInterpLocation InterpLoc4 {InterpComp4, 0};	
	InterpLocations.Add(InterpLoc4);
	
	FInterpLocation InterpLoc5 {InterpComp5, 0};	
	InterpLocations.Add(InterpLoc5);
	
	FInterpLocation InterpLoc6 {InterpComp6, 0};	
	InterpLocations.Add(InterpLoc6);

}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;
	
	for(int32 i = 1; i < InterpLocations.Num(); ++i)
	{
		if(InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}
void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if(Amount < -1 || Amount > 1) return;

	if(InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

void AShooterCharacter::ResetPickupSoundTimer()
{
	bShouldPlayPickupSound = true;
}
void AShooterCharacter::ResetEquipSoundTimer()
{
	bShouldPlayEquipSound = true;
}


void AShooterCharacter::StartPickupSoundTimer()
{
	bShouldPlayPickupSound = false;
	GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AShooterCharacter::ResetPickupSoundTimer, PickupSoundResetTime);
}
void AShooterCharacter::StartEquipSoundTimer()
{
bShouldPlayEquipSound = false;
	GetWorldTimerManager().SetTimer(EquipSoundTimer, this, &AShooterCharacter::ResetEquipSoundTimer, EquipSoundResetTime);
}

void AShooterCharacter::FKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 0) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
}
void AShooterCharacter::OneKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 1) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
}
void AShooterCharacter::TwoKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 2) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);	
}
void AShooterCharacter::ThreeKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 3) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);	
}
void AShooterCharacter::FourKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 4) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);	
}
void AShooterCharacter::FiveKeyPressed()
{
	if(EquippedWeapon->GetSlotIndex() == 5) return;
	ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
}

void AShooterCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if((CurrentItemIndex != NewItemIndex) && (NewItemIndex < Inventory.Num())  && (CombatState == ECombatState::ECS_Unoccupied || CombatState == ECombatState::ECS_Equipping))
	{
		if(bAiming)
		{
			StopAim();
		}
		auto OldEquippedWeapon = EquippedWeapon;
		auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);

		EquipWeapon(NewWeapon);
		OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
		NewWeapon->SetItemState(EItemState::EIS_Equipped);

		CombatState = ECombatState::ECS_Equipping;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if(AnimInstance && EquipMontage) 
		{
			AnimInstance->Montage_Play(EquipMontage);
			AnimInstance->Montage_JumpToSection(TEXT("Equip"));
		}
		NewWeapon->PlayEquipSound(true);
	}
}
void AShooterCharacter::FinishEquipping()
{
	if(CombatState == ECombatState::ECS_Stunned) return;
	CombatState = ECombatState::ECS_Unoccupied;
	if(bAimingButtonPressed)
	{
		Aim();
	}
}

int32 AShooterCharacter::GetEmptyInventorySlot()
{
	for(int32 i = 0; i < Inventory.Num(); ++i)
	{
		if(Inventory[i] == nullptr)
		{
			return i;
		}
	}
	if(Inventory.Num() < INVENTORY_CAPACITY)
	{
		return Inventory.Num();
	}

	/**
	 * @brief Inventory is full
	 * 
	 */
	return -1;
}

void AShooterCharacter::HighlightInventorySlot()
{
	const int32 EmptySlot {GetEmptyInventorySlot()};
	HighlightIconDelegate.Broadcast(EmptySlot, true);
	HighlightedSlot = EmptySlot;
}

void AShooterCharacter::UnHighlightInventorySlot()
{
	HighlightIconDelegate.Broadcast(HighlightedSlot, false);
	HighlightedSlot = -1;
}

EPhysicalSurface AShooterCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start {GetActorLocation()};
	const FVector End {Start + FVector(0.0f, 0.0f, -400.0f)};
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true; 
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}

float AShooterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if(Health - DamageAmount <= 0.0f)
	{
		Health = 0.0f;
		Die();
		auto EnemyController = Cast<AEnemyController>(EventInstigator);
		if(EnemyController)
		{
			EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CharacterDead"), true);
			EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), nullptr);
		}
	}
	else
	{
		Health -= DamageAmount;	
	}

	return DamageAmount;
}

void AShooterCharacter::EndStun()
{
	CombatState = ECombatState::ECS_Unoccupied;
	if(bAimingButtonPressed)
	{
		Aim();
	}
}

void AShooterCharacter::Stun()
{
	if(FMath::IsNearlyZero(Health)) return;

	CombatState = ECombatState::ECS_Stunned;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

void AShooterCharacter::Die()
{
	bDead = true;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}
}

void AShooterCharacter::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if(PlayerController)
	{
		DisableInput(PlayerController);
	}
}