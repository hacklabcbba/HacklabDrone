classdef time
    %TIME Summary of this class goes here
    %   Detailed explanation goes here
    
    properties
        seconds@uint32 scalar
        fraction@uint32 scalar
    end
    
    methods
        function this = time(arg)
            if nargin > 0
                if isa(arg, 'uint8')
                    if numel(arg) == 8
                        var = typecast(arg, 'uint32');
                        this.seconds = var(1);
                        this.fraction = var(2);
                    end
                end
            end
        end
        function cell = ToCell(this)
            cell = {this.seconds this.fraction};
        end
    end
end
