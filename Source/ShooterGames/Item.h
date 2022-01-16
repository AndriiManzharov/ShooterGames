// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "Item.generated.h"

class UBoxComponent;
class UWidgetComponent;
class USphereComponent;
class UCurceFloat;
class AShooterCharacter;
class USoundCue;
class UCurveVector;
UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_PickUp UMETA(DisplayName = "PickUp"),
	EIS_EquipInterping UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp UMETA(DisplayName = "PickedUp"),
	EIS_Equipped UMETA(DisplayName = "Equipped"),
	EIS_Falling UMETA(DisplayName = "Falling"),


	EIS_MAX UMETA(DisplayName = "DefaultMax")
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo UMETA(DisplayName = "Ammo"),
	EIT_Weapon UMETA(DisplayName = "Weapon"),

	EIT_MAX UMETA(DisplayName = "DefaultMax")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;

};

class UDataTable;

UCLASS()
class SHOOTERGAMES_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComp,
						 AActor* OtherActor, UPrimitiveComponent* OtherComp,
						 int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComp,
							AActor* OtherActor, UPrimitiveComponent* OtherComp,
							int32 OtherBodyIndex);
	
	void SetActiveStars();

	virtual void SetItemProperties(EItemState State);

	/** Вызывается по завершению таймера */
	void FinishInterping();

	void ItemInterp(float DeltaTime);

	FVector GetInterpLocation();

	void PlayPickupSound(bool bForcePlaySound = false);

	virtual void InitializeCustomDepth();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void EnableGlowMaterial();

	void UpdatePulse();

	void ResetPulseTimer();
	void StartPulseTimer();
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Вызывается из класса персонажа
	 * 
	 */
	void PlayEquipSound(bool bForcePlaySound = false);

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	virtual void DisableGlowMaterial();

private:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AreaSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemZCurve;

	/** Позиция предмета перед началом анамации взятия */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	/** Позиция перед камерой */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	bool bInterping;
	/** Запускается когда начинается анимация взятия предмета */
	FTimerHandle ItemInterpTimer;

	/** Задержка для таймера */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	AShooterCharacter* Character;

	/** X и Y координаты для анимации перемещения */
	float ItemInterpX;
	float ItemInterpY;

	/** Разница поворота У камеры и У предмета */
	float InterpInitialYawOffset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	USoundCue* PickupSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	USoundCue* EquipSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocIndex;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex;

	/** Изменяется вов время выполениня*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;
	
	/** Используется совместно с UMaterialInstanceDynamic*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	bool bCanChangeCustomDepth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* PulseCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* InterpPulseCurve;

	FTimerHandle PulseTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties", meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconItem;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconAmmo;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

	/**
	 * @brief true когда инвентарь персонажа заполнен 
	 * 
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	bool bCharacterInventoryFull;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* ItemRarityDataTable; 

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor GlowColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor LightColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	FLinearColor DarkColor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	int32 NumberOfStars;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	UTexture2D* IconBackground;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rarity", meta = (AllowPrivateAccess = "true"))
	int32 CustomDepthStencil;
public:

	void SetItemState(EItemState State);
	/** Будет вызываться из  AShooterCharacter*/
	void StartItemCurve(AShooterCharacter* Char, bool bForcePlaySound = false);


	FORCEINLINE UWidgetComponent* GetPickupWidget() const {return PickupWidget;}
	FORCEINLINE USphereComponent* GetAreaSphere() const {return AreaSphere;}
	FORCEINLINE UBoxComponent* GetCollisionBox() const {return CollisionBox;}
	FORCEINLINE EItemState GetItemState() const {return ItemState;}
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const {return ItemMesh;}
	FORCEINLINE UMaterialInstance* GetMaterialInstance() const {return MaterialInstance;}
	FORCEINLINE UMaterialInstanceDynamic* GetDynamicMaterialInstance() const {return DynamicMaterialInstance;}
	FORCEINLINE USoundCue* GetPickupSound() const { return PickupSound;}
	FORCEINLINE FLinearColor GetGlowColor() const {return GlowColor;}
	FORCEINLINE int32 GetMaterialIndex() const {return MaterialIndex;}
	
	
	FORCEINLINE void SetPickupSound(USoundCue* Sound) {PickupSound = Sound;}
	FORCEINLINE void SetEquipSound(USoundCue* Sound) {EquipSound = Sound;}
	FORCEINLINE void SetItemName(FString Name) {ItemName = Name;}
	FORCEINLINE void SetItemIcon(UTexture2D* Icon) {IconItem = Icon;}
	FORCEINLINE void SetAmmoIcon(UTexture2D* Icon) {IconAmmo = Icon;}
	FORCEINLINE void SetMaterialInstance(UMaterialInstance* Instance) {MaterialInstance = Instance;}
	FORCEINLINE void SetDynamicMaterialInstance(UMaterialInstanceDynamic* Instance)  {DynamicMaterialInstance = Instance;}
	FORCEINLINE USoundCue* GetEquipSound() const { return EquipSound;}
	FORCEINLINE int32 GetItemCount() const {return ItemCount;}
	FORCEINLINE int32 GetSlotIndex() const {return SlotIndex;}
	FORCEINLINE void SetSlotIndex(int32 Index) {SlotIndex = Index;}
	FORCEINLINE void SetCharacter(AShooterCharacter* Char) {Character = Char;}
	FORCEINLINE void SetCharacterInventoryFull(bool bFull) {bCharacterInventoryFull = bFull;}
	FORCEINLINE void SetMaterialIndex(int32 Index) {MaterialIndex = Index;}
};
