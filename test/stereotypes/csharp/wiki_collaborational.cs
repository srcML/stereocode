// Method-Stereotypes wiki, Collaborational, C# examples (PowerShell), each in a minimal class

public class OutCommandInner
{
    private string _printerName;

    // The cast leaves the local that the method created, so the method is also a factory
    // @stereotype property factory collaborator
    private LineOutput InstantiateLineOutputInterface()
    {
        PrinterLineOutput printOutput = new(_printerName);
        return (LineOutput)printOutput;
    }
}

public class FileSystemProviderTests
{
    private readonly string testPath;

    private static ExecutionContext GetExecutionContext()
    {
        return null;
    }

    private static ProviderInfo GetProvider()
    {
        return null;
    }

    // @stereotype controller
    public void TestClearContent()
    {
        FileSystemProvider fileSystemProvider = new FileSystemProvider();
        ProviderInfo providerInfoToSet = GetProvider();
        fileSystemProvider.SetProviderInformation(providerInfoToSet);
        fileSystemProvider.Context = new CmdletProviderContext(GetExecutionContext());
        fileSystemProvider.ClearContent(testPath);
        Assert.Empty(File.ReadAllText(testPath));
    }

    // The wiki shows this example under wrapper with the label controller; File.Delete is a static call
    // @stereotype wrapper
    protected virtual void Dispose(bool disposing)
    {
        if (disposing)
        {
            File.Delete(testPath);
        }
    }
}
