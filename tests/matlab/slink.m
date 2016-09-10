classdef slink < handle
    %UNTITLED3 Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(Constant = true, Access = private)
        LengthMax = 1000
        InitChecksum = uint32(hex2dec('00000000'))
        Preamble = struct('Sop', uint8(['s', 'l']), 'Reserved', uint8([255, 255]))
    end
    
    properties(Access = public)
        Identifier@uint16 scalar
        Payload@uint8 vector
    end
    
    properties(SetAccess = private, GetAccess = public)
        Packet@uint8 vector
    end
    
    properties(Access = private)
        Context = struct('Length', 0, 'Identifier', 0, 'Control', struct('State', 1, 'DataIdx', 0))
        CRC = crc32
    end
    
    methods
        function this = slink(~)
            this.InitMessage();
        end
        function this = InitMessage(this)
            this.Context.Length = 0;
            this.Context.Identifier = 0;
            this.Context.Control.State = 1;
            this.Context.Control.DataIdx = 0;
            this.Identifier = 0;
            this.Payload = uint8([]);
            this.Packet = uint8([]);
        end
        function EndMessage(this)
            preamble = [this.Preamble.Sop, this.Preamble.Reserved];
            header = typecast([uint16(numel(this.Payload)), this.Identifier], 'uint8');
            payload = this.Payload;
            
            packet = [header, payload];
            checksum = this.CRC.Calculate(this.InitChecksum, packet);
            checksum = typecast(bitcmp(checksum), 'uint8');
            this.Packet = [preamble, packet, checksum];
        end
        function LoadPayload(this, data)
            assert(isnumeric(data), 'Value must be ''numeric vector''.');
            this.Payload = [this.Payload, typecast(data, 'uint8')];
        end
        function [result, rest] = ReceiveMessage(this, data)
            assert(isa(data, 'uint8'), 'Value must be ''uint8 vector''.');
            rest = uint8([]); result = 0;
            packet = this.Packet;
            state = this.Context.Control.State;
            dataIdx = this.Context.Control.DataIdx;
            len = this.Context.Length;
            identifier = this.Context.Identifier;
            for ii = 1:numel(data)
                newByte = data(ii);
                switch state
                    case 1,
                        % Sop byte 1
                        if newByte == this.Preamble.Sop(1)
                            packet = newByte;
                            packet(8) = uint8(0);
                            state = 2;
                        end
                    case 2,
                        % Sop byte 2
                        if newByte == this.Preamble.Sop(2)
                            packet(2) = newByte;
                            state = 3;
                        else
                            rest = data(ii + 1:end); result = -1;
                            state = 1;
                            break
                        end
                    case 3,
                        % Reserved byte 1
                        packet(3) = newByte;
                        state = 4;
                    case 4,
                        % Reserved byte 3
                        packet(4) = newByte;
                        state = 5;
                    case 5,
                        % Size byte 1
                        packet(5) = newByte;
                        state = 6;
                    case 6,
                        % Size byte 2
                        packet(6) = newByte;
                        len = typecast(packet(5:6), 'uint16');
                        if len <= this.LengthMax
                            packet(8 + len + 4) = uint8(0);
                            state = 7;
                        else
                            rest = data(ii + 1:end); result = -1;
                            state = 1;
                            break
                        end
                    case 7,
                        % Identifier byte 1
                        packet(7) = newByte;
                        state = 8;
                    case 8,
                        % Identifier byte 2
                        packet(8) = newByte;
                        identifier = typecast(packet(7:8), 'uint16');
                        dataIdx = 0;
                        state = 9;
                    case 9,
                        % Data bytes
                        packet(9 + dataIdx) = newByte;
                        dataIdx = dataIdx + 1;
                        if dataIdx >= len + 4
                            % Calculate checksum
                            checksum = this.CRC.Calculate(this.InitChecksum, packet(5:end));
                            
                            % Test checksum
                            if bitcmp(checksum) == 0
                                this.Identifier = identifier;
                                this.Payload = packet(9:8 + len);
                                rest = data(ii + 1:end); result = 1;
                            else
                                rest = data(ii + 1:end); result = -1;
                            end
                            state = 1;
                            break
                        end
                    otherwise
                        rest = data(ii:end); result = -1;
                        state = 1;
                        break
                end
            end
            this.Packet = packet;
            this.Context.Control.State = state;
            this.Context.Control.DataIdx = dataIdx;
            this.Context.Length = len;
            this.Context.Identifier = identifier;
        end
    end
end
