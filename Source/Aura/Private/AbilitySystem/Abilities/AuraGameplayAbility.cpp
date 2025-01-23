// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

FString UAuraGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>%s Level: </><Level>%d</>"), L"Default Ability Name - Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nullam elit lectus, efficitur vel augue in, pellentesque consectetur dolor. Mauris sem odio, vestibulum eget metus quis, tristique mattis lectus. Quisque sit amet purus ullamcorper, tincidunt massa ut, mattis purus. Aliquam auctor ex nibh, non sagittis enim porttitor ut. Sed tincidunt tincidunt sem, aliquet consequat arcu aliquam ac. Suspendisse eu justo eu sapien facilisis pellentesque. Integer quis varius dolor. Ut dapibus enim sit amet metus tincidunt faucibus. Etiam dictum ornare justo nec consequat. Donec mollis molestie iaculis. Phasellus at sollicitudin tortor. Donec a leo elit.", Level);
}

FString UAuraGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more damage. </>"), Level);
}

FString UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Spell Locked Until Level: %d</>"), Level);
}
