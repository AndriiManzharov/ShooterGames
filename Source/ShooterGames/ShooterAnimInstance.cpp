// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon.h"
UShooterAnimInstance::UShooterAnimInstance() :
    Speed(0.0f),
    bIsInAir(false),
    bIsAcceleration(false),
    MovementOffsetYaw(0.0f),
    LastMovementOffsetYaw(0.0f),
    bAiming(false),
    TIPCharacterYaw(0.0f),
    TIPCharacterYawLastFrame(0.0f),
    CharacterRotation(FRotator::ZeroRotator),
    CharacterRotationLastFrame(FRotator::ZeroRotator),
    YawDelta(0.0f),
    RootOffset(0.0f),
    Pitch(0.0f),
    bReloading(false),
    OffsetState(EOffsetState::EOS_Hip),
    bCrouching(false),
    EquipWeaponType(EWeaponType::EWT_MAX),
    bShouldUseFABRIK(false)
{

}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Character = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if(Character == nullptr)
    {
        Character = Cast<AShooterCharacter>(TryGetPawnOwner());
    }
    if(Character)
    {
        bCrouching = Character->GetCrouching();
        bReloading = Character->GetCombatState() == ECombatState::ECS_Reloading;
        bShouldUseFABRIK = Character->GetCombatState() == ECombatState::ECS_Unoccupied 
                        || Character->GetCombatState() == ECombatState::ECS_FireTimerInProgress; 
        FVector Velocity {Character->GetVelocity()};
        Velocity.Z = 0.0f;
        Speed = Velocity.Size();

        bIsInAir = Character->GetMovementComponent()->IsFalling();

        if(Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f)
        {
            bIsAcceleration = true;
        }
        else
        {
            bIsAcceleration = false;
        }
        FRotator AimRotation = Character->GetBaseAimRotation();
        

        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Character->GetVelocity());
        MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
        
        if(Character->GetVelocity().Size() > 0.0f)
        {
            LastMovementOffsetYaw = MovementOffsetYaw; 
        }
        
        bAiming = Character->GetAiming();

        if(bReloading)
        {
            OffsetState = EOffsetState::EOS_Reloading;
        }
        else if (bIsInAir)
        {
            OffsetState = EOffsetState::EOS_InAir;
        }
        else if(Character->GetAiming())
        {
             OffsetState = EOffsetState::EOS_Aiming;
        }
        else
        {
             OffsetState = EOffsetState::EOS_Hip;   
        }

        if(Character->GetEquippedWeapon())
        {
            EquipWeaponType = Character->GetEquippedWeapon()->GetWeaponType();
        }
    }
    TurnInPlace();
    Lean(DeltaTime);
}

void UShooterAnimInstance::TurnInPlace()
{
    if(Character == nullptr) return;
    Pitch = Character->GetBaseAimRotation().Pitch;
    if(Speed > 0 || bIsInAir)
    {
        // Персонаж находится в движении
        RootOffset = 0.0f;
        
        TIPCharacterYaw = Character->GetActorRotation().Yaw;
        TIPCharacterYawLastFrame = TIPCharacterYaw;

        RotationCurve = 0.0f;
        RotationCurveLastFrame = 0.0f;
    }
    else
    {
        TIPCharacterYawLastFrame = TIPCharacterYaw;
        TIPCharacterYaw = Character->GetActorRotation().Yaw;
        const float TIPYawDelta {TIPCharacterYaw - TIPCharacterYawLastFrame};

        // Не дает выйти за промежуток [-180;180] 
        RootOffset = UKismetMathLibrary::NormalizeAxis(RootOffset - TIPYawDelta);
        /**
         * @brief Если в текущей анимации присутствует анимации с кривой
         *        которая называеться Turning получим 1 (так как кривая 
         *        всегда имеет значение 1) или 0 если такой нет.
         */
        const float Turning {GetCurveValue(TEXT("Turning"))};
        if(Turning > 0.0f)
        {
            RotationCurveLastFrame = RotationCurve;
            RotationCurve = GetCurveValue(TEXT("Rotation"));

            const float DeltaRotation {RotationCurve - RotationCurveLastFrame};

            /**
             * @brief Если RootOffset > 0 мы поворачиваемся влево
             *        если больше вправо 
             */

            RootOffset > 0 ? RootOffset -=DeltaRotation : RootOffset +=DeltaRotation;

            const float AbsolutRootYawOffset {FMath::Abs(RootOffset)};
            if(AbsolutRootYawOffset > 90.0f)
            {
                const float YawExcess { AbsolutRootYawOffset - 90.0f};
                RootOffset > 0 ? RootOffset -= YawExcess : RootOffset += YawExcess;
            }
        }
    }


}

void UShooterAnimInstance::Lean(float DeltaTime)
{
    if(Character == nullptr) return;

    CharacterRotationLastFrame = CharacterRotation;
    CharacterRotation = Character->GetBaseAimRotation();

    const FRotator Delta {UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame)};


    const float Target { Delta.Yaw / DeltaTime };

    const float Interp {FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.0f)};

    YawDelta = FMath::Clamp(Interp, -90.0f, 90.0f);


}