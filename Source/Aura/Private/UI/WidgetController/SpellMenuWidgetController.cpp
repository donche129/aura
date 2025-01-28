// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
	SpellPointsChangedDelegate.Broadcast(GetAuraPlayerState()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetAuraAbilitySystemComponent()->AbilityStatusChangedDelegate.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& AbilityStatusTag, int32 NewLevel)
	{
		if (SelectedAbility.AbilityTag.MatchesTagExact(AbilityTag))
		{
			SelectedAbility.AbilityStatusTag = AbilityStatusTag;
			bool bEnableSpendPointsButton = false;
			bool bEnableEquipButton = false;
			ShouldEnableButtons(AbilityStatusTag, CurrentSpellPoints, bEnableSpendPointsButton, bEnableEquipButton);
			FString Description;
			FString NextLevelDescription;
			GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(AbilityInfo, AbilityTag, Description, NextLevelDescription);
			SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointsButton, bEnableEquipButton, Description, NextLevelDescription);
		}
		
		if (AbilityInfo)
		{
			FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = AbilityStatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});

	GetAuraAbilitySystemComponent()->AbilityEquippedDelegate.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

	GetAuraPlayerState()->OnSpellPointsChangedDelegate.AddLambda([this](int32 SpellPoints)
	{
		SpellPointsChangedDelegate.Broadcast(SpellPoints);
		CurrentSpellPoints = SpellPoints;

		bool bEnableSpendPointsButton = false;
		bool bEnableEquipButton = false;
		ShouldEnableButtons(SelectedAbility.AbilityStatusTag, CurrentSpellPoints, bEnableSpendPointsButton, bEnableEquipButton);
		FString Description;
		FString NextLevelDescription;
		GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(AbilityInfo, SelectedAbility.AbilityTag, Description, NextLevelDescription);
		SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointsButton, bEnableEquipButton, Description, NextLevelDescription);
	});
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityTypeTag = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTypeTag;
		StopWaitingForEquipSelectionDelegate.Broadcast(SelectedAbilityTypeTag);
		bWaitingForEquipSelection = false;
	}
	
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	const int32 SpellPoints = GetAuraPlayerState()->GetSpellPoints();
	FGameplayTag AbilityStatusTag;
	
	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	const FGameplayAbilitySpec* AbilitySpec = GetAuraAbilitySystemComponent()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatusTag = GameplayTags.Abilities_Status_Locked;
	}
	else
	{
		AbilityStatusTag = AuraAbilitySystemComponent->GetStatusFromSpec(*AbilitySpec);
	}

	SelectedAbility.AbilityTag = AbilityTag;
	SelectedAbility.AbilityStatusTag = AbilityStatusTag;
	bool bEnableSpendPointsButton = false;
	bool bEnableEquipButton = false;
	ShouldEnableButtons(AbilityStatusTag, SpellPoints, bEnableSpendPointsButton, bEnableEquipButton);
	FString Description;
	FString NextLevelDescription;
	GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(AbilityInfo, AbilityTag, Description, NextLevelDescription);
	SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPointsButton, bEnableEquipButton, Description, NextLevelDescription);
}

void USpellMenuWidgetController::SpendPointButtonPressed()
{
	if (GetAuraAbilitySystemComponent())
	{
		GetAuraAbilitySystemComponent()->ServerSpendSpellPoint(SelectedAbility.AbilityTag);
	}
}

void USpellMenuWidgetController::GlobeDeselect()
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityTypeTag = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTypeTag;
		StopWaitingForEquipSelectionDelegate.Broadcast(SelectedAbilityTypeTag);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.AbilityTag = FAuraGameplayTags::Get().Abilities_None;
	SelectedAbility.AbilityStatusTag = FAuraGameplayTags::Get().Abilities_Status_Locked;

	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityTypeTag = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTypeTag;

	WaitForEquipSelectionDelegate.Broadcast(AbilityTypeTag);
	bWaitingForEquipSelection = true;
	
	/**
	 * The SelectedSlotTag is not actually being used in the current implementation.
	 * See https://www.udemy.com/course/unreal-engine-5-gas-top-down-rpg/learn/lecture/39869226#questions/22569649
	 */
	const FGameplayTag SelectedStatusTag = GetAuraAbilitySystemComponent()->GetStatusFromAbilityTag(SelectedAbility.AbilityTag);
	if (SelectedStatusTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		SelectedSlotTag = GetAuraAbilitySystemComponent()->GetInputTagFromAbilityTag(SelectedAbility.AbilityTag);
	}
}

void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityTypeTag)
{
	if (!bWaitingForEquipSelection) return;
	// Check selected ability against the slot's ability type.
	// Don't equip an offensive ability in a passive ability slot and vice versa.
	const FGameplayTag SelectedAbilityTypeTag = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTypeTag;
	if (!SelectedAbilityTypeTag.MatchesTagExact(AbilityTypeTag)) return;

	GetAuraAbilitySystemComponent()->ServerEquipAbility(SelectedAbility.AbilityTag, SlotTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& AbilityStatusTag, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	bWaitingForEquipSelection = false;

	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
	
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PreviousSlot is a valid slot. Only if equipping an already equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = AbilityStatusTag;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);

	StopWaitingForEquipSelectionDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityTypeTag);
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);
	GlobeDeselect();
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatusTag, int32 SpellPoints, bool& bShouldEnableSpendPointsButton, bool& bShouldEnableEquipButton)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

	bShouldEnableSpendPointsButton = false;
	bShouldEnableEquipButton = false;
	if (AbilityStatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpendPointsButton = true;
		}
	}
	else if (AbilityStatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		if (SpellPoints > 0)
		{
			bShouldEnableSpendPointsButton = true;
		}
	}
	else if (AbilityStatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpendPointsButton = true;
		}
	}
}
