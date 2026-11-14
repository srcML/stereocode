package view;

import model.*;

public class Label {
    private Tag tag;

    // An import of every type of a package reaches the class as well
    // @stereotype property collaborator
    public String text() { return tag.getName(); }
}
