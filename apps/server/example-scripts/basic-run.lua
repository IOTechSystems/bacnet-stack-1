

function Update() --Run every server tick
    
end

function Run() --Run once

    bacnet.createAnalogInputs(10000)
    bacnet.createAnalogOutputs(10000)
    bacnet.createAnalogValues(10000)
    bacnet.createBinaryInputs(10000)
    bacnet.createBinaryOutputs(10000)
    bacnet.createBinaryValues(10000)
    bacnet.createIntegerValues(10000)
    bacnet.createPositiveIntegerValues(10000)
    bacnet.createAccumulators(10000)

    bacnet.setAnalogInput(5000, 1337.00)
    bacnet.setAnalogOutput(5000, 42.00, 1)
    bacnet.setAnalogValue(5000, 1337.00, 1)
    bacnet.setBinaryInput(5000, 1)
    bacnet.setBinaryOutput(5000, 1, 1)
    bacnet.setBinaryValue(5000, 0)
    bacnet.setIntegerValue(5000, -1337, 1)
    bacnet.setPositiveIntegerValue(5000, 42, 1)
    bacnet.setAccumulator(5000, 2222)

    bacnet.setAnalogInputName(5000, "Test analog input")
    bacnet.setAnalogOutputName(5000, "Test analog out")
    bacnet.setAnalogValueName(5000, "Test analog value")
    bacnet.setBinaryInputName(5000, "test B I")
    bacnet.setBinaryOutputName(5000, "test B O")
    bacnet.setBinaryValueName(5000, "test B V")
    bacnet.setIntegerValueName(5000, "test integer value")
    bacnet.setPositiveIntegerValueName(5000, "test positive integer")
    bacnet.setAccumulatorName(5000, "test accumulator")

end
