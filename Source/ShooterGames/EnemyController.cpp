// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"
    #include "BehaviorTree/BehaviorTree.h"
    #include "BehaviorTree/BlackboardComponent.h"  
    #include "BehaviorTree/BehaviorTreeComponent.h"
#include "Enemy.h"

AEnemyController::AEnemyController()
{
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
    check(BlackboardComponent);

    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");


}
void AEnemyController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if(InPawn == nullptr) return;

    AEnemy* Enemy = Cast<AEnemy>(InPawn);
    if(Enemy)
    {
        if(Enemy->GetBehaviorTree())
        {
            BlackboardComponent->InitializeBlackboard(*(Enemy->GetBehaviorTree()->BlackboardAsset));
        }
    }
}