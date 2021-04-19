
function Update() --Run every server tick

end

function sleep(time)
    local duration = os.time() + time
    while os.time() < duration do end
end

function Run() --Run once

    bacnet.resizeAnalogInputObjectArray(5000) 
    
    while true do --every 1 seconds create a new reading

        local val = 42 * math.sin( 0.1 * os.time() )
        for i=0,5000 do
            
            bacnet.setAnalogInput(i, val)
        end
        sleep(1)
    end

end

--Run on each created instance of script
--Use to initialise global variables
