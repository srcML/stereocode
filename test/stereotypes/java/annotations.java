// An annotation names neither the type of a variable nor a variable, whatever arguments the annotation takes

class Profile {
    private int age;
    private @Size(max = 3) int rank;
    private String nick;

    // @stereotype set
    void setAge(@Range(min = 0, max = 150) int age) { this.age = age; }

    // @stereotype set
    void rename(@Named("nick") String value) { this.nick = value; }

    // @stereotype set
    void setPlain(@NonNull String value) { this.nick = value; }

    // @stereotype get
    public @Size(max = 5) String nick() { return nick; }

    // @stereotype get
    int rank() { return rank; }

    // @stereotype set
    void setQualified(@javax.annotation.Nullable String value) { this.nick = value; }

    // @stereotype predicate
    public @Checked(by = "caller") boolean isAdult() { return age > 17; }

    // @stereotype incidental
    int half(int total) { @Scaled(by = 2) int part = total / 2; return part; }
}

class Meter {
    private int value;

    // The element value of the annotation is no data member
    // @stereotype incidental
    int limit() { @Min(value = 1) int count = 2; return count; }

    // @stereotype get
    int value() { return value; }
}
