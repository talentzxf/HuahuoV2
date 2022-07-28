package online.huahuo.backend.db;

import lombok.Data;
import lombok.extern.java.Log;

import javax.persistence.*;

@Entity
@Data
@Log
public class UserDB {
    @Id
    @Column(name = "id", nullable = false)
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Long id;

    private String identifier; // Might be a phone number, might be email, but be external ref.
    private String password;
}
