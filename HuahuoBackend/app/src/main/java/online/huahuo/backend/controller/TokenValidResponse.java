package online.huahuo.backend.controller;

import lombok.Data;

@Data
public class TokenValidResponse{
    private String userName;
    private Boolean isValid;
}
