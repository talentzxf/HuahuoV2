package online.huahuo.backend.controller;

import lombok.Data;
import org.springframework.http.HttpStatus;

@Data
public class LoginStatus{
    private String userName;
    private String failReason;
    private String jwtToken;
    private HttpStatus httpStatus;
}