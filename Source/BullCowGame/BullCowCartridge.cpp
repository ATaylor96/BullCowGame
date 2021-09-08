
#include "BullCowCartridge.h"
#include "Kismet/GameplayStatics.h"
#include "Components/ActorComponent.h"
#include "Components/TextRenderComponent.h"
#include "Console/Terminal.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Actor.h"


void UBullCowCartridge::BeginPlay() // When the game starts
{
    Super::BeginPlay();

    FBullCowCount Count;

    const FString WordListPath = FPaths::ProjectContentDir() / TEXT("WordLists/HiddenWordList.txt");
    FFileHelper::LoadFileToStringArrayWithPredicate(Isograms, *WordListPath, [](const FString& Word)
    {
        return Word.Len() >= 4 && Word.Len() <= 8 && IsIsogram(Word);
    });

    SetupComponents();
    SetupGame();
}

void UBullCowCartridge::SetupComponents() 
{
    // Find Timer and Level Text Render Component
    TArray<UActorComponent*> Components = GetOwner()->GetComponentsByClass(UTextRenderComponent::StaticClass()); 

    for (UActorComponent* Comp : Components)
    {
        if (Comp->GetName() == "TimerTextRender")
        {
            TimerText = Cast<UTextRenderComponent>(Comp);
        }
        else if (Comp->GetName() == "LevelTextRender")
        {
            LevelText = Cast<UTextRenderComponent>(Comp);
        }
    }

    // Auto activate terminal
    Terminal = GetOwner()->FindComponentByClass<UTerminal>();

    if (Terminal != nullptr)
    {
        Terminal->ActivateTerminal();
    }
}

void UBullCowCartridge::OnInput(const FString& PlayerInput) // When the player hits enter
{
    if (bGameOver)
    {
        CurrentLevel = 1;
        ClearScreen();
        SetupGame();
    }
    else if (bNextLevel)
    {
        ClearScreen();
        SetupGame();
    }
    else // Check player guess
    {
        ProcessGuess(PlayerInput);
    }
}

void UBullCowCartridge::SetupGame()
{
    HiddenWord = Isograms[FMath::RandRange(0, Isograms.Num() - 1)];

    // Welcoming the Player
    PrintLine(TEXT("Welcome to...           DEBUG: %s"), *HiddenWord);
    const char* IntroChar = " _    _          _____ _  ________ _____  \n| |  | |   /\\   / ____| |/ /  ____|  __ \\ \n| |__| |  /  \\ | |    | ' /| |__  | |__) |\n|  __  | / /\\ \\| |    |  < |  __| |  _  / \n| |  | |/ ____ \\ |____| . \\| |____| | \\ \\ \n|_|  |_/_/    \\_\\_____|_|\\_\\______|_|  \\_\\ \n";
    FString IntroString(IntroChar);
    PrintLine(TEXT("%s"), *IntroString);

    Lives = HiddenWord.Len();
    bGameOver, bNextLevel = false;
    TimeLeft = MaxTime;

    GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &UBullCowCartridge::Countdown, 1.0f, true);

    LevelText->SetText(FText::FromString("Level " + FString::FromInt(CurrentLevel) + ": " + FString::FromInt(HiddenWord.Len()) + " Characters"));
    TimerText->SetText(FText::FromString("1:00"));

    PrintLine(TEXT("Guess the password. You have %i attempts."), HiddenWord.Len(), Lives);
    PrintLine(TEXT("Type in your guess and press Enter... \n")); // Prompt player for guess
}

void UBullCowCartridge::EndGame(FString Reason, bool bFailed)
{
    if (bFailed)
    {
        bGameOver = true;
        // FAILED
        PrintLine(TEXT(" ______      _____ _      ______ _____  \n|  ____/\\   |_   _| |    |  ____|  __ \\ \n| |__ /  \\    | | | |    | |__  | |  | |\n|  __/ /\\ \\   | | | |    |  __| | |  | |\n| | / ____ \\ _| |_| |____| |____| |__| |\n|_|/_/    \\_\\_____|______|______|_____/ "));
        PrintLine(TEXT("\n%s\nYou have been locked out."), *Reason);
        PrintLine(TEXT("The password was: %s"), *HiddenWord);
        PrintLine(TEXT("\nPress Enter to exit."));
    }
    else if (CurrentLevel >= MaxLevel)
    {
        bGameOver = true;
        // SUCCESS
        PrintLine(TEXT(" _    _          _____ _  ________ _____  \n| |  | |   /\\   / ____| |/ /  ____|  __ \\ \n| |__| |  /  \\ | |    | ' /| |__  | |  | |\n|  __  | / /\\ \\| |    |  < |  __| | |  | |\n| |  | |/ ____ \\ |____| . \\| |____| |__| |\n|_|  |_/_/    \\_\\_____|_|\\_\\______|_____/ \n"));
        PrintLine(TEXT("\nYou're in. Grab the files and log off."));
        PrintLine(TEXT("\nPress Enter to exit."));
    }
    else
    {
        bNextLevel = true;
        ++CurrentLevel;
        PrintLine(TEXT("\nPress Enter to continue..."));
    }
}

void UBullCowCartridge::ProcessGuess(const FString& Guess)
{
    ClearScreen();
    if (Guess == HiddenWord)
    {
        ClearScreen();
        if (SuccessSFX != nullptr)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), SuccessSFX, GetOwner()->GetActorLocation(), 3.0f);
        }
        PrintLine(TEXT("\n\n\n\n\n\n\n\n\nLogin successful... Files secured."));
        EndGame(TEXT(""), false);
        return;
    }

    // Check if correct number of characters
    if (Guess.Len() != HiddenWord.Len())
    {
        PrintLine(TEXT("The hidden word is %i letters long."), HiddenWord.Len());
        PrintLine(TEXT("Sorry, try guessing again!\nYou have %i lives remaining."), Lives);
        return;
    }

    // Check if Isogram
    if (!IsIsogram(Guess))
    {
        PrintLine(TEXT("No repeating letters, guess again!"));
        return;
    }

    // Remove a Life
    if (ErrorSFX != nullptr)
	{
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), ErrorSFX, GetOwner()->GetActorLocation());
    }
    // INVALID
    PrintLine(TEXT(" _______      __     _      _____ _____  \n|_   _\\ \\    / /\\   | |    |_   _|  __ \\ \n  | |  \\ \\  / /  \\  | |      | | | |  | |\n  | |   \\ \\/ / /\\ \\ | |      | | | |  | |\n _| |_   \\  / ____ \\| |____ _| |_| |__| |\n|_____|   \\/_/    \\_\\______|_____|_____/ "));
    PrintLine(TEXT("\nThe password you entered was incorrect.\n"));
    --Lives;

    if (Lives <= 0)
    {
        ClearScreen();
        EndGame(TEXT("Too many attempts."), true);
        return;
    }

    // Show the player Bulls and Cows
    FBullCowCount Score = GetBullCows(Guess);

    PrintLine(TEXT("You have %i Bulls and %i Cows"), Score.Bulls, Score.Cows);
    PrintLine(TEXT("Guess again, you have %i lives left."), Lives);
    PrintLine(TEXT("%i attempts remaining..."), Lives);
}

void UBullCowCartridge::Countdown() 
{
    FString Remaining = "";
    --TimeLeft;

    if (TimeLeft <= 0)
    {
        GetWorld()->GetTimerManager().PauseTimer(CountdownTimerHandle);
        TimeLeft = 0;
        Remaining = "0:00";
        EndGame(TEXT("You ran out of time."), true);
    }
    else
    {
        Remaining = "0:" + FString::FromInt(TimeLeft); 
    }

    TimerText->SetText(FText::FromString(Remaining));
}

bool UBullCowCartridge::IsIsogram(const FString& Word)
{
    for (int32 Index = 0; Index < Word.Len(); Index++)
    {
        for (int32 Comparison = Index + 1; Comparison < Word.Len(); Comparison++)
        {
            if (Word[Index] == Word[Comparison])
            {
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> UBullCowCartridge::GetValidWords(const TArray<FString>& WordList) const
{
    TArray<FString> ValidWords;

    for (FString Word : WordList)
    {
        if (Word.Len() >= 4 && Word.Len() <= 8 && IsIsogram(Word))
        {
            ValidWords.Emplace(Word);
        }
    }

    return ValidWords;
}

FBullCowCount UBullCowCartridge::GetBullCows(const FString& Guess) const
{
    FBullCowCount Count;

    for (int32 GuessIndex = 0; GuessIndex < Guess.Len(); GuessIndex++)
    {
        if (Guess[GuessIndex] == HiddenWord[GuessIndex])
        {
            Count.Bulls++;
            continue;
        }
        for (int32 HiddenIndex = 0; HiddenIndex < HiddenWord.Len(); HiddenIndex++)
        {
            if (Guess[GuessIndex] == HiddenWord[HiddenIndex])
            {
                Count.Cows++;
                break;
            }
        }
    }
    return Count;
}