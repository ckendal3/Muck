// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "../ActionTypes.h"
#include "MuckAround/ActionSystem/ActionTags.h"
#include "MuckAround/ActionSystem/Interfaces/TaggableInterface.h"
#include "UObject/Object.h"
#include "ActionDefinition.generated.h"

class UActionConfig;
class UActionComponent;

// Used to determine the type of limitation on timer based actions.
UENUM(BlueprintType)
enum ELimiterType
{
	LT_Limited    UMETA(DisplayName = "Limited"),
	LT_Unlimited  UMETA(DisplayName = "Unlimited")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FActionEvent, class UActionDefinition*, Action, FGameplayTag, NewTag);

/**
 * The base class for actions. This class should be extend and overridden.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, Within="Actor")
class UActionDefinition : public UObject, public ITaggableInterface
{
	GENERATED_BODY()

public:

	// We want to give the ActionComponent exclusive access
	friend UActionComponent;

	UActionDefinition();

	// The owning action component of this action.
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_OwningComponent, Category = "Action")
		UActionComponent* OwningComponent;

	// Identifier used for searching/indexing and activation
	// EX: Jump, Shoot, Crouch, Perk1, etc
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
		FGameplayTag Identifier;

	// Associated values for finding/matching multiple types of action definitions
	// EX: Movement, Character, Weapon, Damage Over Time, Upgrade, etc
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
		FGameplayTagContainer SortingTags;

	// The tags that are applied via UpdateTags() when this action is active.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action|Tags")
		FGameplayTagContainer OperatingTags;

	// The tags that can block this action from becoming active.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action|Tags")
		FGameplayTagContainer BlockingTags;
	
	// Event that gets broadcasted everytime there is a state change of the action. I.E. Start, Tick Action, End
	UPROPERTY(BlueprintAssignable, Category = "Action|Events")
		FActionEvent OnActionEventListener;
	
	// Used to manage the end action - one off actions should use 'Immediate'
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action")
		EActionEndType ActionEndType = EActionEndType::AET_Immediate;
	
	// Determines if the action is Unlimited (runs indefinite) or should stop itself (limited).
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "ActionEndType == EActionEndType::AET_Timer"), Category = "Action|Timing")
		TEnumAsByte<ELimiterType> LimitType = ELimiterType::LT_Limited;

	// The time duration between each time the PerformAction function is called.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "ActionEndType == EActionEndType::AET_Timer", ClampMin="0"), Category = "Action|Timing")
		float TickRate = 1.f;
	
	// The max amount of times this action should occur before ending. NOTE: Programmers, this internally will be 0 if its set to Unlimited.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "LimitType == ELimiterType::LT_Limited && ActionEndType == EActionEndType::AET_Timer", ClampMin="0"), Category = "Action|Timing")
		int MaxTicks = 1;

	// The current use count of the action. Only changes during gameplay LimiterType == Limited
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (EditCondition = "LimitType == ELimiterType::LT_Limited && ActionEndType == EActionEndType::AET_Timer"), Category = "Action|Timing")
		int TickCount = 0;

#if WITH_EDITORONLY_DATA
	
	// This is an editor only value. Formula is (MaxCount * TimeBetweenOccurence). This is only important for ActionEndType being Timer based
	UPROPERTY(VisibleDefaultsOnly, meta = (EditCondition = "LimitType == ELimiterType::LT_Limited && ActionEndType == EActionEndType::AET_Timer", EditConditionHides), Category = "Action|Timing")
		float TotalTime = 0.f;

#endif
	
	// The current state of the action. Running or Stop will be the only value typically used during runtime.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Action|State", ReplicatedUsing = OnRep_CurrentStateTag, Transient)
		FGameplayTag CurrentStateTag = TAG_ACTION_END;

	// Called immediately after the Action is created for setup. NOTE: This can be overridden and usually only called on server.
	virtual void InitializeAction(UActionComponent* Owner);
	
	UFUNCTION(BlueprintCallable)
		virtual void SetupFromConfig(UActionConfig* InConfig);

	// Resets the Tick Count to 0. NOTE: This can be overridden 
	virtual void ResetCount();
	
	// Returns true if the action is currently running (CurrentStateTag is not "END")
	UFUNCTION(BlueprintPure)
		bool IsActive() const { return CurrentStateTag.MatchesTagExact(TAG_ACTION_END) == false; }

	// Returns if this object is locally controlled
	UFUNCTION(BlueprintPure)
		bool IsLocallyOwned() const;

	UPROPERTY()
	FTimerHandle   TimerHandle;
	FTimerDelegate TimerDelegate;

protected:
	
	// INTERNAL FUNCTIONS //
	
	void Internal_BeginAction();
	
	UFUNCTION()
	void Internal_TickAction();

	void Internal_CustomStateAction(FGameplayTag CustomTag);
	
	void Internal_EndAction();

	void UpdateActiveTags();
	
	UFUNCTION(BlueprintNativeEvent)
		void BeginAction();

	UFUNCTION(BlueprintNativeEvent)
		void TickAction();

	UFUNCTION(BlueprintNativeEvent)
		void CustomStateAction();

	UFUNCTION(BlueprintNativeEvent)
		void EndAction();
	
	UFUNCTION(BlueprintPure)
		AActor* GetOwningActor() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
		virtual void Destroy();

	UFUNCTION(BlueprintNativeEvent)
	bool CanStart() const;
	
	// Calls the internal end action function 
	UFUNCTION(BlueprintCallable, DisplayName = "EndAction")
		void Request_InternalEnd();

	UFUNCTION()
		virtual void OnRep_CurrentStateTag(FGameplayTag PreviousTag);

	// Only clients will run this logic unless explicitly called on server functions.
	UFUNCTION(BlueprintImplementableEvent)
		void OnReceiveNewState(FGameplayTag PreviousTag);
	
	// Occurs on client when an owning component is replicated. Useful for local initialization.
	UFUNCTION()
		virtual void OnRep_OwningComponent();
	
	void NetworkStateRequest(FGameplayTag NewState);
	
	UFUNCTION(Server, Reliable)
		void Server_R_SetActionState(FGameplayTag NewState);

	UFUNCTION(NetMulticast, Reliable)
		void NetMulticast_R_SetActionState(FGameplayTag NewState);
	
	// TAGGABLE INTERFACE FUNCTIONS //

	void SetTag_Implementation(FGameplayTag InTag) override;

	// ENGINE FUNCTIONS //
	
	virtual bool IsSupportedForNetworking() const override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;
	virtual bool CallRemoteFunction(UFunction* Function, void* Params, struct FOutParmRec* OutParams, FFrame* Stack) override;

	virtual UWorld* GetWorld() const override;

#if WITH_EDITOR
	virtual void PreEditChange(FProperty* PropertyAboutToChange) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};