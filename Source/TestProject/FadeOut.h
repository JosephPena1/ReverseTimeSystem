// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "FadeOut.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TESTPROJECT_API UFadeOut : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFadeOut();

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION(BlueprintCallable)
	void OnOverlapEnter(AActor* OverlappedActor, AActor* Other);
	UFUNCTION()
	void OnOverlapEnd(AActor* OverlappedActor, AActor* Other);

	UFUNCTION()
	void Reset();

private:
	AActor* Actor = nullptr;

	float FallTimerDelay = 2.0f;
	float MaxFallTime = 2.0f;
	bool b_IsFalling = false;
	bool b_IsEntered = false;

};
