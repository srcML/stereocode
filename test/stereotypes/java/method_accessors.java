// Accessors (Method-Stereotypes wiki, Structural Accessor): get, predicate, property, and void-accessor

class Thermometer {
    private double celsius;
    private boolean calibrated;

    // @stereotype get
    public double getCelsius() { return celsius; }

    // @stereotype get
    public boolean isCalibrated() { return calibrated; }

    // @stereotype predicate
    public boolean isFreezing() { return celsius <= 0.0; }

    // @stereotype property
    public double getFahrenheit() { return celsius * 9.0 / 5.0 + 32.0; }

    // @stereotype void-accessor
    public void readCelsius(double[] values) { values[0] = celsius; }
}
