classdef control
    %UNTITLED Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        time@time scalar = time
        data@uint32 = zeros(1, 8, 'uint32')
    end
    
    methods
        function cell = ToCell(this)
            cell = {cell2mat(this.time.ToCell) this.data(1:8)};
        end
    end
end
