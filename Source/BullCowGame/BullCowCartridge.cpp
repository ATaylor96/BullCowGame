
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

void UBullCowCartridge::SetupGame()
{
    HiddenWord = Isograms[FMath::RandRange(0, Isograms.Num() - 1)];
    Lives = HiddenWord.Len();
    TimeLeft = MaxTime;
    bNextLevel = false;
    bGameOver = false;

    UE_LOG(LogTemp, Warning, TEXT("%s"), *HiddenWord); // DEBUG

    // Welcoming the Player
    PrintLine(TEXT("Welcome to..."));

    // HACKER
    const char* IntroChar = " _    _          _____ _  ________ _____  \n| |  | |   /\\   / ____| |/ /  ____|  __ \\ \n| |__| |  /  \\ | |    | ' /| |__  | |__) |\n|  __  | / /\\ \\| |    |  < |  __| |  _  / \n| |  | |/ ____ \\ |____| . \\| |____| | \\ \\ \n|_|  |_/_/    \\_\\_____|_|\\_\\______|_|  \\_\\ \n";
    FString IntroString(IntroChar);
    PrintLine(TEXT("%s"), *IntroString);

    PrintLine(TEXT("Guess the password. You have %i attempts."), HiddenWord.Len(), Lives);
    PrintLine(TEXT("Type in your guess and press Enter... \n")); // Prompt player for guess

    // Dislay Level and Timer text
    LevelText->SetText(FText::FromString("Level " + FString::FromInt(CurrentLevel) + ": " + FString::FromInt(HiddenWord.Len()) + " Characters"));
    TimerText->SetText(FText::FromString("1:00"));

    // Setup Countdown timer
    GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, this, &UBullCowCartridge::Countdown, 1.0f, true);
}

void UBullCowCartridge::OnInput(const FString& PlayerInput) // When the player hits enter
{
    if (bNextLevel || bGameOver)
    {
        ClearScreen();
        SetupGame();
    }
    else
    {
        ProcessGuess(PlayerInput);
    }
}

void UBullCowCartridge::EndGame(FString Reason, bool bFailed)
{
    bGameOver = true;
    bNextLevel = false;
    if (bFailed)
    {
        // FAILED
        PrintLine(TEXT(" ______      _____ _      ______ _____  \n|  ____/\\   |_   _| |    |  ____|  __ \\ \n| |__ /  \\    | | | |    | |__  | |  | |\n|  __/ /\\ \\   | | | |    |  __| | |  | |\n| | / ____ \\ _| |_| |____| |____| |__| |\n|_|/_/    \\_\\_____|______|______|_____/ "));
        PrintLine(TEXT("\n%s\nYou have been locked out."), *Reason);
        PrintLine(TEXT("The password was: %s"), *HiddenWord);
        PrintLine(TEXT("\nPress Enter to try again..."));
        CurrentLevel = 0;
    }
    else if (CurrentLevel >= MaxLevel)
    {
        // SUCCESS
        PrintLine(TEXT(" _    _          _____ _  ________ _____  \n| |  | |   /\\   / ____| |/ /  ____|  __ \\ \n| |__| |  /  \\ | |    | ' /| |__  | |  | |\n|  __  | / /\\ \\| |    |  < |  __| | |  | |\n| |  | |/ ____ \\ |____| . \\| |____| |__| |\n|_|  |_/_/    \\_\\_____|_|\\_\\______|_____/ \n"));
        PrintLine(TEXT("\nYou're in! Grab the files and log off."));
        PrintLine(TEXT("\nPress Enter to log off."));
        CurrentLevel = 0;
    }
    else
    {
        bGameOver = false;
        bNextLevel = true;
        PrintLine(TEXT("\nPress Enter to continue..."));
        ++CurrentLevel;
    }
}

void UBullCowCartridge::ProcessGuess(const FString& Guess)
{
    ClearScreen();
    if (Guess == HiddenWord)
    {
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

    // Check if Lives is 0
    if (Lives <= 0)
    {
        ClearScreen();
        EndGame(TEXT("Too many attempts."), true);
        return;
    }

    // Show the player Bulls and Cows
    PrintLine(TEXT("You guessed %i letters... %s were in the right place."), GetCows(Guess), *GetBulls(Guess));

    PrintLine(TEXT("Guess again, you have %i lives left."), Lives);
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

FString UBullCowCartridge::GetBulls(const FString& Guess) const
{
    FString Bulls = "";

    for (int32 GuessIndex = 0; GuessIndex < Guess.Len(); GuessIndex++)
    {
        if (Guess[GuessIndex] == HiddenWord[GuessIndex])
        {
            Bulls += Guess[GuessIndex];
            continue;
        }
        else
        {
            Bulls += "_";
        }
    }
    return *Bulls;
}

int32 UBullCowCartridge::GetCows(const FString& Guess) const
{
    int32 Cows = 0;

    for (int32 GuessIndex = 0; GuessIndex < Guess.Len(); GuessIndex++)
    {
        for (int32 HiddenIndex = 0; HiddenIndex < HiddenWord.Len(); HiddenIndex++)
        {
            if (Guess[GuessIndex] == HiddenWord[HiddenIndex])
            {
                Cows++;
                break;
            }
        }
    }
    return Cows;
}