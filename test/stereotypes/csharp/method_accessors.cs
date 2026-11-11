// Accessors (Method-Stereotypes wiki, Structural Accessor): get, predicate, property, and void-accessor

public class Thermometer
{
    private double celsius;
    private bool calibrated;

    // @stereotype get
    public double GetCelsius() { return celsius; }

    // @stereotype get
    public bool IsCalibrated() { return calibrated; }

    // @stereotype predicate
    public bool IsFreezing() { return celsius <= 0.0; }

    // @stereotype property
    public double GetFahrenheit() { return celsius * 9.0 / 5.0 + 32.0; }

    // @stereotype void-accessor
    public void ReadCelsius(out double value) { value = celsius; }

    public double Celsius
    {
        // @stereotype get
        get { return celsius; }
    }
}
