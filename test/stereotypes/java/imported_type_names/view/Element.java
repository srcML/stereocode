package view;

import model.Tag;

public class Element {
    private Tag tag;

    // The call reaches the get of the imported class
    // @stereotype property collaborator
    public String tagName() { return tag.getName(); }
}
