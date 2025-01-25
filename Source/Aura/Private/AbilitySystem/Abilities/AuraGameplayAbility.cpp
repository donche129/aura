// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

#include "AbilitySystem/AuraAttributeSet.h"

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

float UAuraGameplayAbility::GetManaCost(float InLevel) const
{
	float ManaCost = 0.f;
	if (const UGameplayEffect* CostGameplayEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo ModifierInfo : CostGameplayEffect->Modifiers)
		{
			if (ModifierInfo.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				ModifierInfo.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}
	return ManaCost;
}

float UAuraGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;
	if (const UGameplayEffect* CooldownGameplayEffect = GetCooldownGameplayEffect())
	{
		CooldownGameplayEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}
