// An async method returns the value of its task, as bool of async Task<bool>, or no value, as async Task does,
// while a method without async returns the task itself
using System.Threading.Tasks;

public class Loader
{
    private System.IO.StreamReader reader;
    private int loads;

    // @stereotype command collaborator
    public async Task SkipAsync() { await reader.ReadLineAsync(); }

    // @stereotype predicate wrapper
    public async Task<bool> ReadyAsync() { await Task.Yield(); return loads > 0; }

    // @stereotype get wrapper
    public async System.Threading.Tasks.ValueTask<int> CountAsync() { await Task.Yield(); return loads; }

    // @stereotype property non-void-command collaborator
    public Task<string> NextAsync() { return reader.ReadLineAsync(); }
}
