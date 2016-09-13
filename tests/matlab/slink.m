classdef slink < handle
    %UNTITLED3 Summary of this class goes here
    %   Detailed explanation goes here
    
    properties(Constant = true, Access = private)
        LENGTH_MAX = 1000
        INIT_CHECKSUM = uint32(hex2dec('00000000'))
        SOP = uint8(['s', 'l'])
        PREAMBLE = [uint8(['s', 'l']), uint8([255, 255])]
    end
    
    properties(Access = public)
        Identifier@uint16 scalar
        Payload@uint8 vector
    end
    
    properties(SetAccess = private, GetAccess = public)
        Packet@uint8 vector
    end
    
    properties(Access = private)
        CRC = crc32
        Context = struct('Control', struct('State', 0, 'DataIdx', 0), 'Identifier', 0, 'Length', 0)
    end
    
    methods
        function this = slink(~)
            this.InitMessage();
        end
        function this = InitMessage(this)
            this.Context.Control.State = 1;
            this.Context.Control.DataIdx = 0;
            this.Context.Identifier = 0;
            this.Context.Length = 0;
            this.Identifier = 0;
            this.Payload = uint8([]);
            this.Packet = uint8([]);
        end
        function EndMessage(this)
            preamble = this.PREAMBLE;
            payload = this.Payload;
            header = typecast([uint16(numel(payload)), this.Identifier], 'uint8');
            
            packet = [preamble, header, payload];
            checksum = this.CRC.Calculate(this.INIT_CHECKSUM, packet(5:end));
            checksum = typecast(bitcmp(checksum), 'uint8');
            this.Packet = [packet, checksum];
        end
        function LoadPayload(this, data)
            assert(isnumeric(data), 'Value must be ''numeric vector''.');
            this.Payload = [this.Payload, typecast(data, 'uint8')];
        end
        function [result, rest] = ReceiveMessage(this, data)
            assert(isa(data, 'uint8'), 'Value must be ''uint8 vector''.');
            result = 0; rest = uint8([]);
            packet = this.Packet;
            state = this.Context.Control.State;
            dataIdx = this.Context.Control.DataIdx;
            identifier = this.Context.Identifier;
            len = this.Context.Length;
            for ii = 1:numel(data)
                newByte = data(ii);
                switch state
                    case 1,
                        % Sop byte 1
                        if newByte == this.SOP(1)
                            packet = newByte;
                            packet(8) = uint8(0);
                            state = 2;
                        end
                    case 2,
                        % Sop byte 2
                        if newByte == this.SOP(2)
                            packet(2) = newByte;
                            state = 3;
                        else
                            result = -1; rest = data(ii + 1:end);
                            state = 1;
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
                        % Length byte 1
                        packet(5) = newByte;
                        state = 6;
                    case 6,
                        % Length byte 2
                        packet(6) = newByte;
                        len = typecast(packet(5:6), 'uint16');
                        if len <= this.LENGTH_MAX
                            packet(8 + len + 4) = uint8(0);
                            state = 7;
                        else
                            result = -1; rest = data(ii + 1:end);
                            state = 1;
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
                        numBytes = min(numel(data(ii:end)), numel(packet(9 + dataIdx:end)));
                        packet(9 + dataIdx:8 + dataIdx + numBytes) = data(ii:ii + numBytes - 1);
                        dataIdx = dataIdx + numBytes;
                        if dataIdx >= len + 4
                            % Calculate checksum
                            checksum = this.CRC.Calculate(this.INIT_CHECKSUM, packet(5:end));
                            
                            % Test checksum
                            if bitcmp(checksum) == 0
                                this.Identifier = identifier;
                                this.Payload = packet(9:end - 4);
                                result = 1; rest = data(ii + numBytes:end);
                            else
                                result = -1; rest = data(ii + numBytes:end);
                            end
                            state = 1;
                        end
                    otherwise
                        result = -1; rest = data(ii:end);
                        state = 1;
                end
                if result ~= 0
                    break
                end
            end
            this.Packet = packet;
            this.Context.Control.State = state;
            this.Context.Control.DataIdx = dataIdx;
            this.Context.Identifier = identifier;
            this.Context.Length = len;
        end
    end
end
