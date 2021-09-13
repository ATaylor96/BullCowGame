// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextRenderComponent.h"
#include "Console/Cartridge.h"
#include "Console/Terminal.h"
#include "BullCowCartridge.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BULLCOWGAME_API UBullCowCartridge : public UCartridge
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void OnInput(const FString& Input) override;
	void SetupComponents();
	void SetupGame();
	void EndGame(FString Reason, bool bFailed);
	void ProcessGuess(const FString& Guess);
	void Countdown();
	static bool IsIsogram(const FString& Word);
	TArray<FString> GetValidWords(const TArray<FString>& Words) const;
	FString GetBulls(const FString& Guess) const;
	int32 GetCows(const FString& Guess) const;

// Your declarations go below!
private:
	FString HiddenWord;
	int32 Lives;
	int32 CurrentLevel = 1;
	int32 MaxLevel = 5;
	int32 TimeLeft = 60;
	int32 MaxTime = 60;
	bool bGameOver = false;
	bool bNextLevel = false;
	TArray<FString> Level1Words;
	TArray<FString> Level2Words;
	TArray<FString> Level3Words;
	TArray<FString> Level4Words;
	TArray<FString> Level5Words;
	TArray<FString> Isograms;
	FTimerHandle CountdownTimerHandle;

	UTextRenderComponent* LevelText;
	UTextRenderComponent* TimerText;

	UTerminal* Terminal;
	
	UPROPERTY(EditAnywhere)
	USoundBase* KeyboardSFX;
	UPROPERTY(EditAnywhere)
	USoundBase* ErrorSFX;
	UPROPERTY(EditAnywhere)
	USoundBase* SuccessSFX;
};
