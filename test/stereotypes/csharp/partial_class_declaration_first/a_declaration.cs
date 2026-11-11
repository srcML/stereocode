// The part that declares OnChanged comes first in the archive
public partial class Account {
    private int balance;
    partial void OnChanged();
}
