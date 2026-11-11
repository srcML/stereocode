// Method-Stereotypes wiki, Structural Accessors, C# examples (PowerShell), each in a minimal class

using System;
using System.Collections.Generic;

public class CimIndicationWatcher
{
    private string _eventName;

    // @stereotype get
    protected override string GetSourceObjectEventName()
    {
        return _eventName;
    }
}

public class ModuleSpecification
{
    private string nameSpace;

    internal string Namespace
    {
    // @stereotype get
        get
        {
            return this.nameSpace;
        }
    }
}

public class StopJobCommand
{
    private bool _passThru;
    private bool DidUserSuppressTheOperation { get; set; }
    private bool JobHadErrors { get; set; }

    // @stereotype predicate
    internal bool IsPassThruObjectNeeded()
    {
        return (_passThru) && (!this.DidUserSuppressTheOperation) && (!this.JobHadErrors);
    }
}

public class InternalHost
{
    private int NestedPromptCount { get; set; }

    // @stereotype predicate
    internal bool HostInNestedPrompt()
    {
        if (NestedPromptCount > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

public class LoadObjectOperation
{
    private string _value;

    // @stereotype property
    public override string ToString()
    {
        return "LoadObject(" + (_value ?? "null") + ")";
    }
}

public class CimSessionProxyManager
{
    private Dictionary<Guid, CimSessionWrapper> curCimSessionsById;

    // The wiki label is property; the data member is an object of another class
    // @stereotype property collaborator
    internal int GetSessionsCount()
    {
        return this.curCimSessionsById.Count;
    }
}

internal class FuzzyMatcher
{
    internal readonly uint MinimumDistance;

    // The wiki label is void-accessor; the method returns bool
    // @stereotype predicate wrapper
    internal bool IsFuzzyMatch(string candidate, string pattern, out int score)
    {
        score = GetDamerauLevenshteinDistance(candidate, pattern);
        return score <= MinimumDistance;
    }

    internal static int GetDamerauLevenshteinDistance(string string1, string string2)
    {
        return string1.Length + string2.Length;
    }
}
