// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimationDefinition.h"

UAnimSequenceBase* FAnimationSet::GetAnimation(int Index) const
{
	if(Animations.Num() > 0)
	{
		const int MaxIndex = FMath::Max(0, Animations.Num() - 1); 
	
		const int ValidIndex = FMath::Clamp(Index, 0, MaxIndex);

		return Animations[ValidIndex];
	}
	
	return nullptr;
}

UAnimSequenceBase* FAnimationSet::GetAnimationRandom() const
{
	const int RandIndex = FMath::RandRange(0, Animations.Num() - 1);

	return GetAnimation(RandIndex);
}

UAnimSequenceBase* UAnimationDefinition::GetAnimation(FGameplayTag InTag, int Index)
{
	const FAnimationSet& AnimSet = GetAnimationSet(InTag);

	return AnimSet.GetAnimation(Index);
}

UAnimSequenceBase* UAnimationDefinition::GetAnimationRandom(FGameplayTag InTag)
{
	const FAnimationSet& AnimSet = GetAnimationSet(InTag);

	return AnimSet.GetAnimationRandom();
}

FAnimationSet UAnimationDefinition::GetAnimationSet(FGameplayTag InTag)
{	
	if(AnimationMap.Contains(InTag))
	{
		return AnimationMap[InTag];
	}

	return FAnimationSet();
}
