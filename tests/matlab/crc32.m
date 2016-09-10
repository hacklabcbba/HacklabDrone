classdef crc32
    %UNTITLED Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(Constant = true, Access = private)
        Lut@uint32 = crc32.GenerateLut(hex2dec('EDB88320'))
    end
    
    methods(Static = true, Access = private)
        function lut = GenerateLut(Polynomial)
            input = uint32([0:255]);
            output = input;
            for i = 1:8
                shift = bitshift(output, -1);
                output = bitxor(shift, bitand(output, uint32(1)) * Polynomial);
            end
            lut = output;
        end
    end
    
    methods
        function this = crc32(~)
        end
        function checksum = Calculate(this, checksum, data)
            assert(isa(checksum, 'uint32') && isscalar(checksum), 'Value must be ''uint32 scalar''.');
            assert(isa(data, 'uint8'), 'Value must be ''uint8 vector''.');
            lut = this.Lut;
            crc = bitcmp(checksum);
            for ii = 1:numel(data)
                crc = bitxor(crc, uint32(data(ii)));
                tmp1 = uint32(floor(double(crc) / (256)));
                tmp2 = lut(1 + bitand(crc, uint32(255)));
                crc = bitxor(tmp1, tmp2);
            end
            checksum = bitcmp(crc);
        end
    end
end
