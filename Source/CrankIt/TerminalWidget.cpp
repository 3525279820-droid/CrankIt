#include "TerminalWidget.h"
#include "Components/TextBlock.h"

void UTerminalWidget::AddNewLine(const FString& Line)
{
	CurrentText += Line + TEXT("\n");
	if (TerminalText)
	{
		TerminalText->SetText(FText::FromString(CurrentText));
	}
}
