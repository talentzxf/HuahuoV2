package online.huahuo.backend.controller;

import lombok.Data;

@Data
public class UserExistResponse {
    private String username;
    private Boolean exist;
}
