// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h" 
#include "Kismet/GameplayStatics.h"
// Sets default values
AExplosive::AExplosive() :
	Damage(60.0f),
	ExplosiveImpulse(500.0f)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ExplosiveMesh = CreateDefaultSubobject<UStaticMeshComponent>("ExplosiveMesh");
	SetRootComponent(ExplosiveMesh);

	OverlapSphere = CreateDefaultSubobject<USphereComponent>("OverlapSphere");
	OverlapSphere->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AExplosive::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

 void AExplosive::BulletHit_Implementation(FHitResult HitResult, AActor* Shooter, AController* ShooterController)
 {
	if(ExplodeSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	}
	if(ExplodeParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplodeParticles, HitResult.Location, FRotator(0.0f), true);
	}

	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, ACharacter::StaticClass());

	for(AActor* DamagedActor : OverlappingActors)
	{
		UGameplayStatics::ApplyDamage(
			DamagedActor,
			Damage,
			ShooterController,
			Shooter,
			UDamageType::StaticClass()
		);
		auto ShooterCharacter = Cast<ACharacter>(DamagedActor);
		if(ShooterCharacter)
		{
			const FVector ImpulseVector{ExplosiveImpulse * 1.0f, 0.0f, ExplosiveImpulse * 1.0f};
			//ShooterCharacter->GetMesh()->SetWorldRotation(ShooterCharacter->GetMesh()->GetComponentRotation(), false, nullptr, ETeleportType::TeleportPhysics);
			ShooterCharacter->GetMesh()->AddForce(ImpulseVector);
		}
	}

	Destroy();
 }

