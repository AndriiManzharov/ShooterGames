// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletHitInterface.h"
#include "Explosive.generated.h"

class UParticleSystem;
class USoundCue;
class USphereComponent;

UCLASS()
class SHOOTERGAMES_API AExplosive : public AActor, public IBulletHitInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AExplosive();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* ExplodeParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USoundCue* ExplodeSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ExplosiveMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USphereComponent* OverlapSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ExplosiveImpulse;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController) override;
};
