// CodeForgeTemplateEngine.cpp
// Implementation of FCodeForgeTemplateEngine — a minimal, dependency-free
// string template processor for the CodeForge plugin (Layer 3).

#include "CodeForgeTemplateEngine.h"

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void FCodeForgeTemplateEngine::SetTemplateResolver(TFunction<FString(const FString&)> Resolver)
{
    TemplateResolver = MoveTemp(Resolver);
}

FCodeForgeTemplateResult FCodeForgeTemplateEngine::Process(
    const FString& Template,
    const TMap<FString, FString>& Vars) const
{
    FCodeForgeTemplateContext Context;
    Context.Variables = Vars;
    return Process(Template, Context);
}

FCodeForgeTemplateResult FCodeForgeTemplateEngine::Process(
    const FString& Template,
    const FCodeForgeTemplateContext& Context) const
{
    return ProcessInternal(Template, Context, TEXT(""), 0, 1);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{
    /** Count newlines in a substring so we can maintain line numbers. */
    int32 CountNewlines(const FString& Str, int32 From, int32 To)
    {
        int32 Count = 0;
        const int32 Len = FMath::Min(To, Str.Len());
        for (int32 I = From; I < Len; ++I)
        {
            if (Str[I] == TEXT('\n'))
            {
                ++Count;
            }
        }
        return Count;
    }

    /** Build an error result. */
    FCodeForgeTemplateResult MakeTemplateError(
        const FString& Message,
        int32 Line,
        const FString& TemplateName)
    {
        FCodeForgeTemplateResult R;
        R.bSuccess      = false;
        R.ErrorMessage  = Message;
        R.ErrorLine     = Line;
        R.ErrorTemplate = TemplateName;
        return R;
    }
} // anonymous namespace

// ---------------------------------------------------------------------------
// FindMatchingClose
// ---------------------------------------------------------------------------

bool FCodeForgeTemplateEngine::FindMatchingClose(
    const FString& Content,
    int32 SearchFrom,
    const FString& OpenTag,
    const FString& CloseTag,
    int32& OutStart,
    int32& OutEnd) const
{
    int32 Depth = 1;
    int32 Pos   = SearchFrom;
    const int32 ContentLen = Content.Len();

    while (Pos < ContentLen)
    {
        // Check for nested open tag first (same keyword prefix)
        if (Content.Find(OpenTag, ESearchCase::CaseSensitive, ESearchDir::FromStart, Pos) == Pos)
        {
            // Make sure this is actually a fresh occurrence — advance past it
            ++Depth;
            Pos += OpenTag.Len();
            continue;
        }

        // Check for close tag
        if (Content.Find(CloseTag, ESearchCase::CaseSensitive, ESearchDir::FromStart, Pos) == Pos)
        {
            --Depth;
            if (Depth == 0)
            {
                OutStart = Pos;
                OutEnd   = Pos + CloseTag.Len();
                return true;
            }
            Pos += CloseTag.Len();
            continue;
        }

        ++Pos;
    }

    return false; // closing tag not found
}

// ---------------------------------------------------------------------------
// ProcessInternal — the core recursive processing method
// ---------------------------------------------------------------------------

FCodeForgeTemplateResult FCodeForgeTemplateEngine::ProcessInternal(
    const FString& Template,
    const FCodeForgeTemplateContext& Context,
    const FString& TemplateName,
    int32 Depth,
    int32 LineOffset) const
{
    static const int32 MaxIncludeDepth = 10;

    FCodeForgeTemplateResult Result;
    Result.bSuccess = true;

    const int32 TemplateLen = Template.Len();
    if (TemplateLen == 0)
    {
        return Result; // empty template — success with empty output
    }

    FString Output;
    Output.Reserve(TemplateLen * 2);

    int32 Pos      = 0;
    int32 CurLine  = LineOffset;

    while (Pos < TemplateLen)
    {
        // Scan for the next '{{'
        int32 OpenBrace = Template.Find(TEXT("{{"), ESearchCase::CaseSensitive,
                                        ESearchDir::FromStart, Pos);

        if (OpenBrace == INDEX_NONE)
        {
            // No more directives — append the rest as-is
            Output += Template.Mid(Pos);
            break;
        }

        // Append the literal text before this directive
        if (OpenBrace > Pos)
        {
            const FString Literal = Template.Mid(Pos, OpenBrace - Pos);
            CurLine += CountNewlines(Template, Pos, OpenBrace);
            Output  += Literal;
        }
        else
        {
            // Opening brace is at exactly Pos; newlines are counted below
        }

        // Update line for the '{{' itself (no newlines in '{{')
        // Find the closing '}}'
        const int32 DirectiveStart = OpenBrace + 2; // skip '{{'
        int32 CloseBrace = Template.Find(TEXT("}}"), ESearchCase::CaseSensitive,
                                         ESearchDir::FromStart, DirectiveStart);

        if (CloseBrace == INDEX_NONE)
        {
            return MakeTemplateError(
                FString::Printf(TEXT("Unclosed '{{' at line %d"), CurLine),
                CurLine, TemplateName);
        }

        FString Directive = Template.Mid(DirectiveStart, CloseBrace - DirectiveStart).TrimStartAndEnd();
        const int32 AfterDirective = CloseBrace + 2; // skip '}}'

        // ------------------------------------------------------------------
        // {{#if Condition}} ... {{/if}}
        // ------------------------------------------------------------------
        if (Directive.StartsWith(TEXT("#if ")))
        {
            FString ConditionName = Directive.Mid(4).TrimStartAndEnd(); // after "#if "
            bool bNegated = false;

            if (ConditionName.StartsWith(TEXT("!")))
            {
                bNegated = true;
                ConditionName = ConditionName.Mid(1).TrimStartAndEnd();
            }

            // Find matching {{/if}}
            int32 CloseStart, CloseEnd;
            const bool bFound = FindMatchingClose(
                Template, AfterDirective,
                TEXT("{{#if "), TEXT("{{/if}}"),
                CloseStart, CloseEnd);

            if (!bFound)
            {
                return MakeTemplateError(
                    FString::Printf(TEXT("Unclosed '{{#if %s}}' (no matching '{{/if}}') at line %d"),
                        *ConditionName, CurLine),
                    CurLine, TemplateName);
            }

            // Evaluate condition
            const bool* CondPtr = Context.Conditions.Find(ConditionName);
            const bool CondValue = (CondPtr != nullptr) ? *CondPtr : false;
            const bool bInclude  = bNegated ? !CondValue : CondValue;

            if (bInclude)
            {
                // Process the inner block
                const FString InnerBlock = Template.Mid(AfterDirective, CloseStart - AfterDirective);
                const int32   InnerLine  = CurLine + CountNewlines(Template, OpenBrace, AfterDirective);
                FCodeForgeTemplateResult InnerResult =
                    ProcessInternal(InnerBlock, Context, TemplateName, Depth, InnerLine);

                if (!InnerResult.bSuccess)
                {
                    return InnerResult;
                }
                Output += InnerResult.Output;
            }

            // Advance past {{/if}}
            CurLine += CountNewlines(Template, Pos, CloseEnd);
            Pos = CloseEnd;
            continue;
        }

        // ------------------------------------------------------------------
        // {{#each ArrayName}} ... {{/each}}
        // ------------------------------------------------------------------
        if (Directive.StartsWith(TEXT("#each ")))
        {
            const FString ArrayName = Directive.Mid(6).TrimStartAndEnd(); // after "#each "

            // Find matching {{/each}}
            int32 CloseStart, CloseEnd;
            const bool bFound = FindMatchingClose(
                Template, AfterDirective,
                TEXT("{{#each "), TEXT("{{/each}}"),
                CloseStart, CloseEnd);

            if (!bFound)
            {
                return MakeTemplateError(
                    FString::Printf(TEXT("Unclosed '{{#each %s}}' (no matching '{{/each}}') at line %d"),
                        *ArrayName, CurLine),
                    CurLine, TemplateName);
            }

            const FString InnerBlock = Template.Mid(AfterDirective, CloseStart - AfterDirective);
            const int32   InnerLine  = CurLine + CountNewlines(Template, OpenBrace, AfterDirective);

            // Look up the array — absence means empty (not an error)
            const TArray<FCodeForgeTemplateArrayItem>* ArrayPtr = Context.Arrays.Find(ArrayName);

            if (ArrayPtr != nullptr)
            {
                for (const FCodeForgeTemplateArrayItem& Item : *ArrayPtr)
                {
                    // Build a per-iteration context: item values override parent variables
                    FCodeForgeTemplateContext IterContext = Context;
                    for (const TPair<FString, FString>& KV : Item.Values)
                    {
                        IterContext.Variables.Add(KV.Key, KV.Value);
                    }

                    FCodeForgeTemplateResult IterResult =
                        ProcessInternal(InnerBlock, IterContext, TemplateName, Depth, InnerLine);

                    if (!IterResult.bSuccess)
                    {
                        return IterResult;
                    }
                    Output += IterResult.Output;
                }
            }
            // If array not found, produce nothing (empty array semantics)

            // Advance past {{/each}}
            CurLine += CountNewlines(Template, Pos, CloseEnd);
            Pos = CloseEnd;
            continue;
        }

        // ------------------------------------------------------------------
        // {{> template_name}} — include
        // ------------------------------------------------------------------
        if (Directive.StartsWith(TEXT("> ")))
        {
            const FString IncludeName = Directive.Mid(2).TrimStartAndEnd();

            if (!TemplateResolver)
            {
                return MakeTemplateError(
                    FString::Printf(
                        TEXT("No template resolver set; cannot include '%s' at line %d"),
                        *IncludeName, CurLine),
                    CurLine, TemplateName);
            }

            if (Depth >= MaxIncludeDepth)
            {
                return MakeTemplateError(
                    FString::Printf(
                        TEXT("Maximum include depth (%d) exceeded while including '%s' at line %d"),
                        MaxIncludeDepth, *IncludeName, CurLine),
                    CurLine, TemplateName);
            }

            const FString IncludeContent = TemplateResolver(IncludeName);
            if (IncludeContent.IsEmpty())
            {
                return MakeTemplateError(
                    FString::Printf(
                        TEXT("Include template '%s' not found (resolver returned empty string) at line %d"),
                        *IncludeName, CurLine),
                    CurLine, TemplateName);
            }

            FCodeForgeTemplateResult IncludeResult =
                ProcessInternal(IncludeContent, Context, IncludeName, Depth + 1, 1);

            if (!IncludeResult.bSuccess)
            {
                return IncludeResult;
            }

            Output += IncludeResult.Output;

            CurLine += CountNewlines(Template, Pos, AfterDirective);
            Pos = AfterDirective;
            continue;
        }

        // ------------------------------------------------------------------
        // {{/if}} or {{/each}} — unexpected closing tag (no matching open)
        // ------------------------------------------------------------------
        if (Directive == TEXT("/if"))
        {
            return MakeTemplateError(
                FString::Printf(TEXT("Unexpected '{{/if}}' without matching '{{#if}}' at line %d"),
                    CurLine),
                CurLine, TemplateName);
        }
        if (Directive == TEXT("/each"))
        {
            return MakeTemplateError(
                FString::Printf(TEXT("Unexpected '{{/each}}' without matching '{{#each}}' at line %d"),
                    CurLine),
                CurLine, TemplateName);
        }

        // ------------------------------------------------------------------
        // {{VariableName}} — variable substitution
        // ------------------------------------------------------------------
        {
            const FString VarName = Directive; // already trimmed

            const FString* ValuePtr = Context.Variables.Find(VarName);
            if (ValuePtr == nullptr)
            {
                return MakeTemplateError(
                    FString::Printf(
                        TEXT("Unknown variable '{{%s}}' referenced at line %d"),
                        *VarName, CurLine),
                    CurLine, TemplateName);
            }

            Output += *ValuePtr;
            CurLine += CountNewlines(Template, Pos, AfterDirective);
            Pos = AfterDirective;
            continue;
        }
    }

    Result.Output = MoveTemp(Output);
    return Result;
}

