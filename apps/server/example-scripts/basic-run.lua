

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
    bacnet.setAnalogValue(5000, 1337.01, 1)
    bacnet.setBinaryInput(5000, 1)
    bacnet.setBinaryOutput(5000, 1, 1)
    bacnet.setBinaryValue(5000, 1, 1)
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

    ai = bacnet.getAnalogInputPresentValue(5000)
    ao = bacnet.getAnalogOutputPresentValue(5000)
    av = bacnet.getAnalogValuePresentValue(5000)
    bi = bacnet.getBinaryInputPresentValue(5000)
    bo = bacnet.getBinaryOutputPresentValue(5000)
    bv = bacnet.getBinaryValuePresentValue(5000)
    iv = bacnet.getIntegerValuePresentValue(5000)
    piv = bacnet.getPositiveIntegerValuePresentValue(5000)
    acc = bacnet.getAccumulatorPresentValue(5000)
    
    io.write( "AI: " .. tostring(ai) .. "\n")
    io.write( "AO: " .. tostring(ao) .. "\n")
    io.write( "AV: " .. tostring(av) .. "\n")
    io.write( "BI: " .. tostring(bi) .. "\n")
    io.write( "BO: " .. tostring(bo) .. "\n")
    io.write( "BV: " .. tostring(bv) .. "\n")
    io.write( "IV: " .. tostring(iv) .. "\n")
    io.write( "PIV: " .. tostring(piv) .. "\n")
    io.write( "ACC: " .. tostring(acc) .. "\n")

end
