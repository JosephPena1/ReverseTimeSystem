// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeControlComponent.h"

// Sets default values for this component's properties
UTimeControlComponent::UTimeControlComponent()
{
	// turn off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;

}


// Called when the game starts
void UTimeControlComponent::BeginPlay()
{
	Super::BeginPlay();

	Actor = GetOwner();
	Character = (ACharacter*)UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	Mesh = Actor->FindComponentByClass<UStaticMeshComponent>();
	CapsuleComp = Actor->FindComponentByClass<UCapsuleComponent>();

	//Checks if a skeleton mesh is being used
	if (CapsuleComp)
		b_isCapsuleComp = true;

	Timer = Delay;
	FunctionDelay = Delay;
}


// Called every frame
void UTimeControlComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Timer -= DeltaTime;
	FunctionDelay -= DeltaTime;

	//If the game is unpaused, remove everything from the iterator to the end of the array
	if (b_IsDirty)
	{
		TransformArray.RemoveAt(IteratorIndex, TransformArray.Num() - IteratorIndex, true);
		VelocityArray.RemoveAt(IteratorIndex, VelocityArray.Num() - IteratorIndex, true);

		//Reset Iterator, Supposed problems?
		b_IterSet = false;
		b_IsDirty = false;
	}

	//Character Functions
	if (b_isCapsuleComp)
	{
		if (Timer <= 0 && !b_IsPaused)
			NormalTimeCharacter();
		else if (b_IsPaused)
			PauseCharacter();
	}

	//Actor Functions
	else
	{
		if (Timer <= 0 && !b_IsPaused)
			NormalTimeActor();
		else if (b_IsPaused)
			PauseActor();
	}
}

//Toggles b_IsPaused
void UTimeControlComponent::TogglePause()
{
	b_IsPaused = !b_IsPaused;
	b_IsDirty = (IteratorIndex != 0 || b_IterSet);

	if (b_RecentChange)
	{
		Mesh->SetPhysicsLinearVelocity(PhysicsVelocity.GetLocation());
		Mesh->SetPhysicsAngularVelocity(PhysicsVelocity.GetScale3D());
	}
}

//Keeps the actor frozen
int UTimeControlComponent::PauseActor()
{
	if (!b_IterSet)
	{
		IteratorIndex = TransformArray.Num() - 1;
		b_IterSet = true;
	}

	FTransform Transform = TransformArray[IteratorIndex];
	Mesh->SetWorldTransform(Transform);
	//Reduces jittering
	Mesh->SetPhysicsLinearVelocity({ 0, 0, 0 });
	Mesh->SetPhysicsAngularVelocity({ 0, 0, 0 });
	return 0;
}

//Keeps the Character frozen
int UTimeControlComponent::PauseCharacter()
{
	if (!b_IterSet)
	{
		IteratorIndex = TransformArray.Num() - 1;
		b_IterSet = true;
	}

	FTransform Transform = TransformArray[IteratorIndex];
	CapsuleComp->SetWorldTransform(Transform);
	//Reduces jittering
	CapsuleComp->SetPhysicsLinearVelocity({ 0, 0, 0 });
	CapsuleComp->SetPhysicsAngularVelocity({ 0, 0, 0 });
	return 0;
}

//Moves the Actor forwards in time
int UTimeControlComponent::ForwardActor()
{
	if (!Mesh || !b_IsPaused)
		return 1;

	IteratorIndex += 1;
	//Return if at 0
	if (IteratorIndex > TransformArray.Num() - 1)
	{
		IteratorIndex = TransformArray.Num() - 1;
		return 2;
	}

	FTransform Transform = TransformArray[IteratorIndex];

	//Set the Meshs [Transform] and [Linear Velocity]
	Mesh->SetWorldTransform(Transform);
	Mesh->SetPhysicsLinearVelocity(Transform.GetLocation());

	//Sets a [Transform] to the [Velocity] at the index
	FTransform Velocity = VelocityArray[IteratorIndex];
	PhysicsVelocity = Velocity;
	b_RecentChange = true;

	Mesh->SetPhysicsLinearVelocity(Velocity.GetLocation());
	Mesh->SetPhysicsAngularVelocity(Velocity.GetScale3D());

	return 0;
}

//Moves the Character forwards in time
int UTimeControlComponent::ForwardCharacter()
{
	if (!CapsuleComp || !b_IsPaused)
		return 1;

	IteratorIndex += 1;
	//Return if at 0
	if (IteratorIndex > TransformArray.Num())
	{
		IteratorIndex = TransformArray.Num() - 1;
		return 2;
	}

	FTransform Transform = TransformArray[IteratorIndex];

	//Set the Meshs [Transform] and [Linear Velocity]
	CapsuleComp->SetWorldTransform(Transform);
	CapsuleComp->SetPhysicsLinearVelocity(Transform.GetLocation());

	//Sets a [Transform] to the [Velocity] at the index
	FTransform Velocity = VelocityArray[IteratorIndex];

	CapsuleComp->SetPhysicsLinearVelocity(Velocity.GetLocation());
	CapsuleComp->SetPhysicsAngularVelocity(Velocity.GetScale3D());

	return 0;
}

//Reverses Time for Actor
int UTimeControlComponent::ReverseActor()
{
	if (!Mesh || !b_IsPaused)
		return 1;

	if (FunctionDelay <= 0)
	{
		IteratorIndex -= 1;
		//Return if at 0
		if (IteratorIndex < 0)
		{
			IteratorIndex = 0;
			return 2;
		}

		FTransform Transform = TransformArray[IteratorIndex];

		//Set the Meshs [Transform] and [Linear Velocity]
		Mesh->SetWorldTransform(Transform);
		Mesh->SetPhysicsLinearVelocity(Transform.GetLocation());

		//Sets a [Transform] to the [Velocity] at the index
		FTransform Velocity = VelocityArray[IteratorIndex];
		PhysicsVelocity = Velocity;
		b_RecentChange = true;

		Mesh->SetPhysicsLinearVelocity(Velocity.GetLocation());
		Mesh->SetPhysicsAngularVelocity(Velocity.GetScale3D());

		FunctionDelay = Delay;
	}

	return 0;
}

//Reverses Time for Character
int UTimeControlComponent::ReverseCharacter()
{
	if (!CapsuleComp || !b_IsPaused)
		return 1;

	IteratorIndex -= 1;
	//Return if at 0
	if (IteratorIndex < 0)
	{
		IteratorIndex = 0;
		return 2;
	}

	FTransform Transform = TransformArray[IteratorIndex];

	//Set the Meshs [Transform] and [Linear Velocity]
	CapsuleComp->SetWorldTransform(Transform);
	CapsuleComp->SetPhysicsLinearVelocity(Transform.GetLocation());

	//Sets a [Transform] to the [Velocity] at the index
	FTransform Velocity = VelocityArray[IteratorIndex];

	CapsuleComp->SetPhysicsLinearVelocity(Velocity.GetLocation());
	CapsuleComp->SetPhysicsAngularVelocity(Velocity.GetScale3D());

	return 0;
}

//Updates positions in arrays for Actor
void UTimeControlComponent::NormalTimeActor()
{
	if (b_IterSet)
		b_IterSet = false;

	//Continues velocity after exiting reverse mode
	if (b_RecentChange)
	{
		Mesh->SetPhysicsLinearVelocity(PhysicsVelocity.GetLocation());
		Mesh->SetPhysicsAngularVelocity(PhysicsVelocity.GetScale3D());
		b_RecentChange = false;
	}

	//Removes the first index if the max amount is reached
	if (TransformArray.Num() == MaxPositions && MaxPositions > 0)
	{
		TransformArray.RemoveAt(0, 1, true);
		VelocityArray.RemoveAt(0, 1, true);
	}

	//Adds the current transform into the [Transform Array]
	TransformArray.Add(Mesh->GetComponentTransform());

	//Adds the current velocity into the [Velocity Array]
	FTransform NewVelocity;
	NewVelocity.SetLocation(Mesh->GetPhysicsLinearVelocity());
	NewVelocity.SetScale3D(Mesh->GetPhysicsAngularVelocity());
	NewVelocity.SetRotation(FQuat(0.0f, 0.0f, 0.0f, 0.0f));

	VelocityArray.Add(NewVelocity);
	Timer = Delay;
}

//Updates positions in arrays for Character
void UTimeControlComponent::NormalTimeCharacter()
{
	if (b_IterSet)
		b_IterSet = false;

	//Removes the first index if the max amount is reached
	if (TransformArray.Num() == MaxPositions && MaxPositions > 0)
		TransformArray.RemoveAt(0, 1, true);
	if (VelocityArray.Num() == MaxPositions && MaxPositions > 0)
		VelocityArray.RemoveAt(0, 1, true);

	//Adds the current transform into the [Transform Array]
	TransformArray.Add(CapsuleComp->GetComponentTransform());

	//Adds the current velocity into the [Velocity Array]
	FTransform NewVelocity;
	NewVelocity.SetLocation(CapsuleComp->GetPhysicsLinearVelocity());
	NewVelocity.SetScale3D(CapsuleComp->GetPhysicsAngularVelocity());
	NewVelocity.SetRotation(FQuat(0.0f, 0.0f, 0.0f, 0.0f));

	VelocityArray.Add(NewVelocity);
	Timer = Delay;
}
