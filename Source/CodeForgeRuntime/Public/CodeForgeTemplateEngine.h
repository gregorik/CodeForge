#pragma once

#include "CoreMinimal.h"

// ---------------------------------------------------------------------------
// Data types for the template engine
// ---------------------------------------------------------------------------

/** A single item in an array for {{#each}} loops. */
struct FCodeForgeTemplateArrayItem
{
    /** Variable name → value for this array item. */
    TMap<FString, FString> Values;
};

/** Full context passed to the template engine for rendering. */
struct FCodeForgeTemplateContext
{
    /** Simple variable substitutions for {{VariableName}}. */
    TMap<FString, FString> Variables;

    /** Boolean conditions for {{#if Condition}} / {{#if !Condition}}. */
    TMap<FString, bool> Conditions;

    /** Arrays for {{#each ArrayName}} loops. */
    TMap<FString, TArray<FCodeForgeTemplateArrayItem>> Arrays;
};

/** Result returned by FCodeForgeTemplateEngine::Process. */
struct FCodeForgeTemplateResult
{
    bool bSuccess = false;
    FString Output;
    FString ErrorMessage;
    int32 ErrorLine = -1;

    /** Which template file the error occurred in (empty for the root template). */
    FString ErrorTemplate;
};

// ---------------------------------------------------------------------------
// Template engine class
// ---------------------------------------------------------------------------

/**
 * FCodeForgeTemplateEngine
 *
 * A minimal, dependency-free string template processor that supports:
 *   {{VariableName}}                 - variable substitution
 *   {{#if Condition}}...{{/if}}      - conditional block
 *   {{#if !Condition}}...{{/if}}     - negated conditional
 *   {{#each ArrayName}}...{{/each}}  - loop over array
 *   {{> template_name}}              - include another template
 *
 * This is Layer 3 of the CodeForge architecture. It has zero knowledge of
 * UE5 macros, class types, or code generation specifics.
 */
class CODEFORGERUNTIME_API FCodeForgeTemplateEngine
{
public:

    /**
     * Simple API: process a template with only variable substitutions.
     * Internally converts the TMap to a full FCodeForgeTemplateContext.
     */
    FCodeForgeTemplateResult Process(const FString& Template, const TMap<FString, FString>& Vars) const;

    /**
     * Full API: process a template with the complete context
     * (variables, conditions, and arrays).
     */
    FCodeForgeTemplateResult Process(const FString& Template, const FCodeForgeTemplateContext& Context) const;

    /**
     * Set a resolver function for {{> include_name}} directives.
     * The resolver receives the template name and returns its content.
     * Return an empty string to signal that the template was not found
     * (which will produce an error).
     */
    void SetTemplateResolver(TFunction<FString(const FString&)> Resolver);

private:

    TFunction<FString(const FString&)> TemplateResolver;

    /**
     * Internal recursive processing entry point.
     *
     * @param Template      The template text to process.
     * @param Context       The data context.
     * @param TemplateName  Name of the current template file (for error reporting).
     * @param Depth         Current include-recursion depth (max = 10).
     * @param LineOffset    Line number offset (for nested calls to report correct lines).
     */
    FCodeForgeTemplateResult ProcessInternal(
        const FString& Template,
        const FCodeForgeTemplateContext& Context,
        const FString& TemplateName,
        int32 Depth,
        int32 LineOffset) const;

    /**
     * Find the matching closing tag for an opening block tag.
     *
     * Scans forward through Content starting at SearchFrom, counting nesting
     * depth so that nested same-type blocks are handled correctly.
     *
     * @param Content       Full template text being scanned.
     * @param SearchFrom    Index to start searching from.
     * @param OpenTag       The opening tag string, e.g. "{{#if" or "{{#each".
     * @param CloseTag      The closing tag string, e.g. "{{/if}}" or "{{/each}}".
     * @param OutStart      [out] Start index of the closing tag in Content.
     * @param OutEnd        [out] Index just past the closing tag.
     * @return true if the closing tag was found; false if unclosed.
     */
    bool FindMatchingClose(
        const FString& Content,
        int32 SearchFrom,
        const FString& OpenTag,
        const FString& CloseTag,
        int32& OutStart,
        int32& OutEnd) const;
};

