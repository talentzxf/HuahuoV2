package online.huahuo.backend.db;

import lombok.Data;
import lombok.extern.java.Log;

import javax.persistence.*;
import java.util.Date;

@Entity
@Data
@Log
@Table(name = "USERACCOUNTS")
public class UserDB {
    @Id
    @Column(name = "id", nullable = false)
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    @Column(unique = true)
    private String username; // Might be a phone number, might be email, might be external ref. but need to be unique.

    @Column(name = "password_hash")
    private String password;

    private String nickname;

    private UserRole role;

    private UserStatus status;

    private Date createTime;
    private Date lastLoginTime;
    private Date modifiedTime;
}
