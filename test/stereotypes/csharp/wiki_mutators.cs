// Method-Stereotypes wiki, Structural Mutators, C# examples (PowerShell), each in a minimal class

public class PSDataCollectionStream
{
    private bool _disposed;

    // @stereotype set
    protected virtual void Dispose(bool disposing)
    {
        if (_disposed)
        {
            return;
        }
        _disposed = true;
    }
}

public class Serializer
{
    public TypeTable TypeTable;
}

public class SerializationContext
{
    private Serializer _serializer;

    internal TypeTable TypeTable
    {
        // The wiki label is set; the data member is an object of another class
        // @stereotype set collaborator
        set { _serializer.TypeTable = value; }
    }
}

public class WSManConfigurationOption : PSTransportOption
{
    private uint maxenvelopesizekb;
    private bool maxenvelopesizekbSet;
    private const string nameMaxEnvelopeSizeKB = "MaxEnvelopeSizekb";

    public uint MaxEnvelopeSizeKB
    {
        // The wiki label is non-void-command; a set accessor returns void
        // @stereotype command
        set
        {
            maxenvelopesizekb = value;
            maxenvelopesizekbSet = true;
            base.SetParameter(value, nameMaxEnvelopeSizeKB);
        }
    }
}
