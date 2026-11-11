// Method-Stereotypes wiki, Degenerate, C# examples (PowerShell), each in a minimal class

public class CimIndicationWatcher
{
    // @stereotype incidental
    protected override string GetSourceObjectEventName()
    {
        return "CimIndicationArrived";
    }
}

internal class InternalHost : PSHost
{
    private int _nestedPromptCount;

    // The wiki label is wrapper stateless; the call goes to an overload of the same class
    // @stereotype command
    public override void EnterNestedPrompt()
    {
        EnterNestedPrompt(null);
    }

    internal void EnterNestedPrompt(InternalCommand callingCommand)
    {
        _nestedPromptCount++;
    }
}

public class WSManClientSessionTransportManager
{
    // @stereotype empty
    internal virtual void RemoveCommandTransportManager(Guid powerShellCmdId)
    {

    }
}
